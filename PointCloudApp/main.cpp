#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
#include "MeshShaderTest.h"
#include "SoftwareRasterizer.h"
#include <random>
#include "Profiler.h"
#include "ProcessExecutor.h"
#include "MeshViewer.h"
#include "AIDataGenerator.h"
#include "HalfEdgeStruct.h"
#include "HalfEdgeLoader.h"
#include <filesystem>
#include <fstream>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <limits>

namespace
{
int RunBunnyCheck(const std::filesystem::path& outputDirectory)
{
	try {
		const std::filesystem::path inputPath = R"(E:\cgModel\bunny4000.half)";
		auto require = [](bool condition, const std::string& message) {
			if (!condition) { throw std::runtime_error(message); }
		};
		// Check raw indices before Load calls Set/CreateFace and dereferences them.
		std::ifstream input(inputPath, std::ios::binary);
		input.exceptions(std::ios::failbit | std::ios::badbit);
		int header[4];
		input.read(reinterpret_cast<char*>(header), sizeof(header));
		const int vertices = header[1], edgeCount = header[2], faces = header[3];
		static_assert(sizeof(int) == 4 && sizeof(float) == 4 && sizeof(KI::HalfEdge) == 20);
		require(header[0] == 2 && vertices > 0 && faces > 0 && edgeCount >= vertices &&
			static_cast<int64_t>(edgeCount) == 3LL * faces, "Invalid triangular .half header.");
		require(std::filesystem::file_size(inputPath) == 16ULL + 16ULL * vertices +
			20ULL * edgeCount + 4ULL * faces, "Unexpected .half file length.");
		input.seekg(16ULL + 16ULL * vertices);
		std::vector<KI::HalfEdge> rawEdges(edgeCount);
		input.read(reinterpret_cast<char*>(rawEdges.data()), 20LL * edgeCount);
		std::vector<int> faceEdges(faces);
		input.read(reinterpret_cast<char*>(faceEdges.data()), 4LL * faces);
		for (int i = 0; i < edgeCount; ++i) {
			const auto& e = rawEdges[i];
			require(e.endPos >= 0 && e.endPos < vertices && e.nextEdge >= 0 && e.nextEdge < edgeCount &&
				e.beforeEdge >= 0 && e.beforeEdge < edgeCount && e.oppositeEdge >= -1 &&
				e.oppositeEdge < edgeCount && e.face >= 0 && e.face < faces,
				"Invalid index at half-edge " + std::to_string(i));
		}
		for (int i = 0; i < faces; ++i) {
			require(faceEdges[i] >= 0 && faceEdges[i] < edgeCount, "Invalid face representative.");
			const auto& e = rawEdges[faceEdges[i]];
			require(e.face == i && rawEdges[e.nextEdge].face == i && rawEdges[e.beforeEdge].face == i &&
				e.nextEdge != faceEdges[i] && e.beforeEdge != faceEdges[i] && e.nextEdge != e.beforeEdge &&
				rawEdges[e.nextEdge].nextEdge == e.beforeEdge && rawEdges[e.beforeEdge].nextEdge == faceEdges[i],
				"Invalid triangle loop at face " + std::to_string(i));
		}
		input.close();
		std::unique_ptr<KI::HalfEdgeStruct> mesh(KI::HalfEdgeLoader::Load(inputPath.string()));
		size_t boundaryEdges = 0;
		for (int i = 0; i < edgeCount; ++i) {
			const auto& e = mesh->GetHalfEdge(i);
			require(rawEdges[e.nextEdge].beforeEdge == i && rawEdges[e.beforeEdge].nextEdge == i,
				"Broken next/previous at half-edge " + std::to_string(i));
			if (e.oppositeEdge == -1) { ++boundaryEdges; }
			else {
				const auto& opposite = rawEdges[e.oppositeEdge];
				require(opposite.oppositeEdge == i && opposite.endPos == rawEdges[e.beforeEdge].endPos &&
					rawEdges[opposite.beforeEdge].endPos == e.endPos,
					"Broken opposite at half-edge " + std::to_string(i));
			}
		}
		for (const auto& p : mesh->GetVertex()) {
			require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z), "Non-finite coordinate.");
		}
		// Geometry experiment: measure every face before attempting remeshing.
		std::vector<float> areas(faces);
		double totalArea = 0;
		float minArea = std::numeric_limits<float>::max(), maxArea = 0;
		int maxFace = 0;
		for (int i = 0; i < faces; ++i) {
			areas[i] = mesh->CalcFaceArea(i);
			require(std::isfinite(areas[i]) && areas[i] > 0, "Invalid area at face " + std::to_string(i));
			totalArea += areas[i];
			minArea = (std::min)(minArea, areas[i]);
			if (areas[i] > maxArea) { maxArea = areas[i]; maxFace = i; }
		}
		require(std::filesystem::create_directory(outputDirectory), "Output directory already exists. Choose a new directory.");
		std::ofstream obj(outputDirectory / "bunny.obj"), csv(outputDirectory / "face-areas.csv");
		obj.exceptions(std::ios::failbit | std::ios::badbit);
		csv.exceptions(std::ios::failbit | std::ios::badbit);
		obj << std::setprecision(9);
		csv << std::setprecision(9) << "face_id,area\n";
		for (const auto& p : mesh->GetVertex()) { obj << "v " << p.x << ' ' << p.y << ' ' << p.z << '\n'; }
		for (int i = 0; i < faces; ++i) {
			const auto& f = mesh->GetIndexedFace(i);
			obj << "f " << f.position[0] + 1 << ' ' << f.position[1] + 1 << ' ' << f.position[2] + 1 << '\n';
			csv << i << ',' << areas[i] << '\n';
		}
		obj.close(); csv.close();
		std::cout << std::setprecision(9) << "PASS Bunny: vertices=" << vertices << " halfEdges=" << edgeCount
			<< " faces=" << faces << " boundaryHalfEdges=" << boundaryEdges
			<< "\nArea: total=" << totalArea << " min=" << minArea << " mean=" << totalArea / faces
			<< " max=" << maxArea << " maxFace=" << maxFace
			<< "\nOutput: " << outputDirectory.string() << std::endl;
		return 0;
	}
	catch (const std::exception& error) {
		std::cerr << "FAIL Bunny: " << error.what() << std::endl;
		return 1;
	}
}

// First harness exercise: known input -> existing geometry code -> assertions -> OBJ.
int RunHalfEdgeCheck(const std::filesystem::path& outputDirectory)
{
	try {
		// A new directory keeps experiments separate and prevents accidental overwrites.
		if (!std::filesystem::create_directory(outputDirectory)) {
			throw std::runtime_error("Output directory already exists. Choose a new directory.");
		}
		const auto inputPath = outputDirectory / "triangle.half";
		const auto outputPath = outputDirectory / "triangle.obj";
		static_assert(sizeof(int) == 4 && sizeof(float) == 4);
		static_assert(sizeof(KI::HalfEdge) == 20);
		{
			std::ofstream input(inputPath, std::ios::binary);
			input.exceptions(std::ios::failbit | std::ios::badbit);
			auto write = [&](const auto& value) {
				input.write(reinterpret_cast<const char*>(&value), sizeof(value));
			};
			// Version 2: four int32 header fields, vertices (3 float32 + int32),
			// half-edges (5 int32), then one representative edge per face.
			for (int value : { 2, 3, 3, 1 }) { write(value); }
			const float positions[3][3] = { {0, 0, 0}, {2, 0, 0}, {0, 1, 0} };
			for (int i = 0; i < 3; ++i) {
				// Loader converts file XYZ to internal ZYX.
				write(positions[i][2]); write(positions[i][1]); write(positions[i][0]);
				write(i);
			}
			const KI::HalfEdge edges[3] = {
				{1, 1, 2, -1, 0}, {2, 2, 0, -1, 0}, {0, 0, 1, -1, 0}
			};
			for (const auto& edge : edges) { write(edge); }
			write(0);
			input.close();
		}

		auto require = [](bool condition, const char* message) {
			if (!condition) { throw std::runtime_error(message); }
		};
		// Only this generated, known-valid fixture is loaded in the first exercise.
		std::unique_ptr<KI::HalfEdgeStruct> mesh(KI::HalfEdgeLoader::Load(inputPath.string()));
		require(mesh && mesh->GetVertexNum() == 3 && mesh->GetEdgeNum() == 3 &&
			mesh->GetFaceNum() == 1, "Unexpected mesh counts.");
		const auto& edges = mesh->GetHalfEdges();
		for (int i = 0; i < 3; ++i) {
			const auto& edge = edges[i];
			require(edge.endPos >= 0 && edge.endPos < 3 &&
				edge.nextEdge >= 0 && edge.nextEdge < 3 &&
				edge.beforeEdge >= 0 && edge.beforeEdge < 3,
				"Half-edge index out of range.");
		}
		for (int i = 0; i < 3; ++i) {
			const auto& edge = edges[i];
			require(edges[edge.nextEdge].beforeEdge == i && edges[edge.beforeEdge].nextEdge == i &&
				edges[edges[edge.nextEdge].nextEdge].nextEdge == i &&
				edge.face == 0 && edge.oppositeEdge == -1, "Triangle connectivity mismatch.");
			const auto& p = mesh->GetVertex(i);
			require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),
				"Non-finite vertex.");
		}
		require(glm::length(mesh->GetVertex(0) - KI::Vector3(0, 0, 0)) < 1e-6f &&
			glm::length(mesh->GetVertex(1) - KI::Vector3(2, 0, 0)) < 1e-6f &&
			glm::length(mesh->GetVertex(2) - KI::Vector3(0, 1, 0)) < 1e-6f,
			"Loaded coordinates differ from the fixture.");

		// Change/extend this geometry operation in the next exercise.
		const float area = mesh->CalcFaceArea(0);
		require(std::isfinite(area) && std::abs(area - 1.0f) < 1e-6f,
			"Expected triangle area = 1.");
		std::ofstream output(outputPath);
		output.exceptions(std::ios::failbit | std::ios::badbit);
		output << std::setprecision(9);
		for (const auto& p : mesh->GetVertex()) {
			output << "v " << p.x << ' ' << p.y << ' ' << p.z << '\n';
		}
		const auto& face = mesh->GetIndexedFace(0);
		output << "f " << face.position[0] + 1 << ' ' << face.position[1] + 1
			<< ' ' << face.position[2] + 1 << '\n';
		output.close();
		std::cout << "PASS HalfEdge: vertices=3 halfEdges=3 faces=1 area=" << area
			<< "\nOBJ: " << outputPath.string() << std::endl;
		return 0;
	}
	catch (const std::exception& error) {
		std::cerr << "FAIL HalfEdge: " << error.what() << std::endl;
		return 1;
	}
}
}

int main(int argc, char* argv[])
{
	if (argc >= 2 && std::string(argv[1]) == "--halfedge-bunny") {
		if (argc != 3) {
			std::cerr << "Usage: PointCloudApp.exe --halfedge-bunny <new-output-directory>\n";
			return 2;
		}
		return RunBunnyCheck(argv[2]);
	}
	if (argc >= 2 && std::string(argv[1]) == "--halfedge-check") {
		if (argc != 3) {
			std::cerr << "Usage: PointCloudApp.exe --halfedge-check <new-output-directory>\n";
			return 2;
		}
		return RunHalfEdgeCheck(argv[2]);
	}
	
	//KI::AIDataGenerator generator;
	//auto data = generator.LoadMeshsegBenchmark("E:\\cgModel\\MeshsegBenchmark-1.0\\data");
	//generator.SaveMeshCNN(data);
	//std::string path = "E:\\cgModel\\bunny6000.half";
	//auto pBunny = std::shared_ptr<KI::HalfEdgeStruct>(KI::HalfEdgeLoader::Load(path));
	//KI::String outPath = "E:\\cgModel\\MeshsegBenchmark-1.0\\data\\predict\\bunny.meshcnn";
	//generator.SaveMeshCNN(outPath, *pBunny);
	//return 0;

	std::cout << std::fixed << std::setprecision(2); // 浮動小数点2桁まで
    
	KI::AIProcessor::Instance().ExecuteASync("--named");
	//KI::RadixSortTest app;
	//KI::PrefixSumTest app;
	//KI::HistogramTest app;
	//KI::ComputeShaderTest app;
	//KI::ComputePointCloudApp app;
	KI::PointCloudApp app;
	//KI::SoftwareRasterizer app;
	//KI::MeshShaderTest app;
	//KI::MeshViewer app;
	app.Initialize();
	app.Execute();
	app.Finalize();


	KI::AIProcessor::Instance().FinalizeASync();
	return 0;
}

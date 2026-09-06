#include "AIDataGenerator.h"
#include "FileUtility.h"
#include "GeometryLoader.h"
#include "PointCloudIO.h"
#include "PointCloud.h"
#include "GeometryUtility.h"
#include "Mesh.h"
#include "Utility.h"
#include "HalfEdgeStruct.h"
#include <stdexcept>
namespace fs = std::filesystem;

namespace KI
{

AIDataGenerator::AIDataGenerator()
{
}

AIDataGenerator::~AIDataGenerator()
{
}

Vector<AIDataFolder> AIDataGenerator::LoadMeshsegBenchmark(const std::filesystem::path& root)
{
	Vector<AIDataFolder> folders;
	const auto offRoot = root / "off";
	const auto segRoot = root / "seg\\Benchmark";

	for (const auto& file : std::filesystem::directory_iterator(offRoot)) {
		if (!file.is_regular_file()) { continue; }
		AIDataFolder folder;
		folder.className = file.path().filename().string();
		AIDataFolder::Data data;
		data.filePath = file.path();

		// 400.off -> 400
		const auto modelName = file.path().stem();

		// seg/Benchmark/400/
		const auto segDir = segRoot / modelName;

		if (std::filesystem::exists(segDir) &&
			std::filesystem::is_directory(segDir)) {
			for (const auto& segFile : std::filesystem::directory_iterator(segDir)) {
				if (!segFile.is_regular_file()) { continue; }
				if (segFile.path().extension() != ".seg") { continue; }
				data.segFile.push_back(segFile.path());
			}
		}

		folder.datas.emplace_back(std::move(data));
		folders.emplace_back(std::move(folder));
	}

	return folders;
}

namespace fs = std::filesystem;
Vector<AIDataFolder> AIDataGenerator::LoadModelNetTrain(const fs::path& root)
{
	Vector<AIDataFolder> classes;

	// airplane, bathtub ...
	for (const auto& classDir : fs::directory_iterator(root)) {
		if (!classDir.is_directory())
			continue;

		fs::path trainDir = classDir.path() / "train";

		if (!fs::exists(trainDir))
			continue;

		AIDataFolder category;
		category.className = classDir.path().filename().string();

		for (const auto& file : fs::directory_iterator(trainDir)) {
			if (!file.is_regular_file())
				continue;

			category.datas.emplace_back();
			category.datas.back().filePath = file.path();
		}

		std::sort(category.datas.begin(), category.datas.end(),
			[](const auto& a, const auto& b)
		{
			return a.filePath < b.filePath;
		});

		classes.emplace_back(std::move(category));
	}

	std::sort(classes.begin(), classes.end(),
		[](const auto& a, const auto& b)
	{
		return a.className < b.className;
	});

	return classes;
}

std::vector<Vector3> segColor;
std::vector<Vector3> AIDataGenerator::LoadMeshsegBenchmarkSeg(const std::filesystem::path& path)
{
	std::ifstream file(path);
	std::vector<Vector3> segments;
	int segment;

	if (segColor.size() == 0) {
		segColor.resize(255);
		for (int i = 0; i < segColor.size(); i++) {
			segColor[i] = Random::Vec3(0, 1);
		}
	}

	while (file >> segment) {
		segments.push_back(segColor[std::clamp(segment, 0, (int)segColor.size() - 1)]);
	}

	return segments;
}

std::vector<Vector3> AIDataGenerator::LoadSegBinary(const std::string& path)
{
	int faceCount = 0;
	std::vector<int32_t> seg;
	std::ifstream ifs(path, std::ios::binary);
	ifs.read(reinterpret_cast<char*>(&faceCount), sizeof(faceCount));
	seg.resize(faceCount);
	ifs.read(reinterpret_cast<char*>(seg.data()), sizeof(int32_t) * faceCount);

	std::vector<Vector3> segments;
	if (segColor.size() == 0) {
		segColor.resize(255);
		for (int i = 0; i < segColor.size(); i++) {
			segColor[i] = Random::Vec3(0, 1);
		}
	}

	for (size_t i = 0; i < seg.size(); i++) {
		segments.push_back(segColor[std::clamp(seg[i], 0, (int)segColor.size() - 1)]);
	}

	return segments;
}


void AIDataGenerator::SavePointCloud(const String& folderName, int sampleCount)
{

    if (sampleCount <= 0) {
        throw std::invalid_argument(
            "sampleCount must be greater than zero.");
    }

    const fs::path rootPath(folderName.c_str());

    if (!fs::exists(rootPath)) {
        throw std::runtime_error(
            "ModelNet40 folder does not exist: " +
            rootPath.string());
    }

    if (!fs::is_directory(rootPath)) {
        throw std::runtime_error(
            "Specified path is not a directory: " +
            rootPath.string());
    }

    /*
        想定構造:

        ModelNet40/
            airplane/
                train/
                test/
            chair/
                train/
                test/
    */
    for (const auto& categoryEntry :
        fs::directory_iterator(rootPath)) {
        if (!categoryEntry.is_directory()) {
            continue;
        }

        const fs::path categoryFolder =
            categoryEntry.path();

        const fs::path trainFolder =
            categoryFolder / "test";

        if (!fs::exists(trainFolder) ||
            !fs::is_directory(trainFolder)) {
            continue;
        }

        ConvertTrainFolder(trainFolder, sampleCount);
    }
}

void AIDataGenerator::ConvertTrainFolder(const fs::path& trainFolder,int sampleCount)
{
    const fs::path categoryFolder = trainFolder.parent_path();
    const std::string outputFolderName = "test_" + std::to_string(sampleCount);
    const fs::path outputFolder = categoryFolder / outputFolderName;
    fs::create_directories(outputFolder);

    for (const auto& entry : fs::recursive_directory_iterator(trainFolder)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const fs::path sourceFile = entry.path();
        std::string extension = sourceFile.extension().string();
        if (extension != ".off") {
            continue;
        }

        /*
            train配下にサブフォルダがあっても、
            相対構造を出力側へ維持する。
        */
        const fs::path relativePath = fs::relative(sourceFile, trainFolder);

        fs::path destinationFile = outputFolder / relativePath;
        destinationFile.replace_extension(PointCloudIO::binExt);

        fs::create_directories(destinationFile.parent_path());

        ConvertFile(sourceFile, destinationFile, sampleCount);
    }
}
void AIDataGenerator::ConvertFile(const fs::path& sourceFile, const fs::path& destinationFile, int sampleCount)
{
	auto mesh = std::unique_ptr<Mesh>(GeometryLoader::LoadOff(sourceFile.string()));
	auto points = MeshAlgorithm::CreateSampleOnFace(*mesh, sampleCount);
	if (points.size() != static_cast<std::size_t>(sampleCount)) {
		throw std::runtime_error("Generated point count does not match sampleCount.");
	}

	PointCloud pointCloud;
	pointCloud.SetPosition(std::move(points));
	PointCloudIO::OutputBinary(&pointCloud, destinationFile.string().c_str());
}

void AIDataGenerator::SaveMeshCNN(const String& filePath, const HalfEdgeStruct& halfEdge)
{
	struct FaceSerializer
	{
		float centerX;
		float centerY;
		float centerZ;
		float normalX;
		float normalY;
		float normalZ;
		float area;
		float edgeLength0;
		float edgeLength1;
		float edgeLength2;
		float angle0;
		float angle1;
		float angle2;
		int32_t neighbor0;
		int32_t neighbor1;
		int32_t neighbor2;
	};

	std::vector<FaceSerializer> faceDatas;
	faceDatas.reserve(halfEdge.GetFaceNum());
	for (auto k = 0; k < halfEdge.GetFaceNum(); k++) {
		const auto center = halfEdge.CalcGravity(k);
		const auto normal = halfEdge.CalcFaceNormal(k);
		const auto face = halfEdge.GetIndexedFace(k);
		FaceSerializer data;
		data.centerX = center.x; data.centerY = center.y; data.centerZ = center.z;
		data.normalX = normal.x; data.normalY = normal.y; data.normalZ = normal.z;
		data.area = halfEdge.CalcFaceArea(k);
		data.edgeLength0 = halfEdge.CalcEdgeLength(face.edge[0]);
		data.edgeLength1 = halfEdge.CalcEdgeLength(face.edge[1]);
		data.edgeLength2 = halfEdge.CalcEdgeLength(face.edge[2]);
		data.angle0 = halfEdge.CalcDihedralAngle(face.edge[0]);
		data.angle1 = halfEdge.CalcDihedralAngle(face.edge[1]);
		data.angle2 = halfEdge.CalcDihedralAngle(face.edge[2]);
		data.neighbor0 = halfEdge.GetOppositeFace(face.edge[0]);
		data.neighbor1 = halfEdge.GetOppositeFace(face.edge[1]);
		data.neighbor2 = halfEdge.GetOppositeFace(face.edge[2]);
		if (data.area <= 1e-4f) {
			data.normalX = 0.0f;
			data.normalY = 0.0f;
			data.normalZ = 0.0f;

			data.area = 0.0f;

			data.angle0 = 0.0f;
			data.angle1 = 0.0f;
			data.angle2 = 0.0f;
		}
		faceDatas.push_back(data);
	}


	std::ofstream ofs(filePath, std::ios::binary | std::ios::out);
	// Header
	const uint32_t faceCount = static_cast<uint32_t>(faceDatas.size());
	ofs.write(reinterpret_cast<const char*>(&faceCount), sizeof(faceCount));
	ofs.write(reinterpret_cast<const char*>(faceDatas.data()), sizeof(FaceSerializer) * faceDatas.size());

}
void AIDataGenerator::SaveMeshCNN(const Vector<AIDataFolder>& folders)
{
	for (auto i = 0; i < folders.size(); i++) {
		const auto& folder = folders[i];
		for (auto j = 0; j < folder.datas.size(); j++) {
			const auto& data = folder.datas[j];
			// E:\cgModel\MeshsegBenchmark-1.0\data\off\xxx.off
			// E:\cgModel\MeshsegBenchmark-1.0\data\meshcnn\xxx.meshcnn
			const auto offDirectory = data.filePath.parent_path();
			const auto dataDirectory = offDirectory.parent_path();
			const auto outputDirectory = dataDirectory / "meshcnn";
			std::filesystem::create_directories(outputDirectory);
			auto outputPath = outputDirectory / data.filePath.filename();
			outputPath.replace_extension(".meshcnn");
			auto mesh = std::unique_ptr<Mesh>(GeometryLoader::LoadOff(data.filePath.string()));
			mesh->Normalize();
			auto halfEdge = HalfEdgeStruct::Create(*mesh);
			SaveMeshCNN(outputPath.string(), halfEdge);
		}
	}
}


}

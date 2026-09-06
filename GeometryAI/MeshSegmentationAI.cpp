#include "MeshSegmentationAI.h"
#include <fstream>
#include <filesystem>
namespace KI
{
MeshSegmentationAI::MeshData MeshSegmentationAI::LoadMeshData(const std::string& path)
{
	MeshData mesh;

	std::ifstream ifs(path, std::ios::binary);
	if (!ifs.is_open()) {
		std::cout << "Failed to open : " << path << std::endl;
		return mesh;
	}

	uint32_t faceCount = 0;

	ifs.read(reinterpret_cast<char*>(&faceCount), sizeof(faceCount));

	if (!ifs || faceCount == 0) {
		return mesh;
	}

	mesh.faces.resize(faceCount);
	ifs.read(reinterpret_cast<char*>(mesh.faces.data()), sizeof(FaceSerializer) * faceCount);
	if (!ifs) {
		std::cout << "Failed to read : " << path << std::endl;
		mesh.faces.clear();
	}

	for (size_t i = 0; i < mesh.faces.size(); ++i) {
		const auto& face = mesh.faces[i];

		const float values[] =
		{
			face.centerX,
			face.centerY,
			face.centerZ,
			face.normalX,
			face.normalY,
			face.normalZ,
			face.area,
			face.edgeLength0,
			face.edgeLength1,
			face.edgeLength2,
			face.angle0,
			face.angle1,
			face.angle2
		};

		bool invalid = false;

		for (float value : values) {
			if (!std::isfinite(value)) {
				invalid = true;
				break;
			}
		}

		if (!invalid) {
			continue;
		}

		std::cout
			<< "Invalid Face[" << i << "] "
			<< "Center=("
			<< face.centerX << ", "
			<< face.centerY << ", "
			<< face.centerZ << ") "
			<< "Normal=("
			<< face.normalX << ", "
			<< face.normalY << ", "
			<< face.normalZ << ") "
			<< "Area=" << face.area << " "
			<< "EdgeLength=("
			<< face.edgeLength0 << ", "
			<< face.edgeLength1 << ", "
			<< face.edgeLength2 << ") "
			<< "Angle=("
			<< face.angle0 << ", "
			<< face.angle1 << ", "
			<< face.angle2 << ") "
			<< "Neighbor=("
			<< face.neighbor0 << ", "
			<< face.neighbor1 << ", "
			<< face.neighbor2 << ")"
			<< std::endl;
	}

	return mesh;
}

std::vector<int64_t> MeshSegmentationAI::LoadSeg(const std::string& path)
{
	std::vector<int64_t> labels;
	std::ifstream ifs(path);
	if (!ifs.is_open()) {
		std::cout << "Failed to open : " << path << std::endl;
		return labels;
	}

	int64_t label = 0;
	while (ifs >> label) { labels.push_back(label); }
	return labels;
}
torch::Tensor MeshSegmentationAI::CreateNeighborTensor(const MeshData& mesh)
{
	std::vector<int64_t> neighbors;
	neighbors.reserve(mesh.faces.size() * 3);

	for (const auto& face : mesh.faces) {
		neighbors.push_back(face.neighbor0);
		neighbors.push_back(face.neighbor1);
		neighbors.push_back(face.neighbor2);
	}

	return torch::from_blob(
		neighbors.data(),
		{
			static_cast<int64_t>(mesh.faces.size()),
			3
		},
		torch::kInt64
	).clone();
}
torch::Tensor MeshSegmentationAI::CreateFeatureTensor(const MeshData& mesh)
{
	std::vector<float> features;
	features.reserve(mesh.faces.size() * 13);

	for (const auto& face : mesh.faces) {
		features.push_back(face.centerX);
		features.push_back(face.centerY);
		features.push_back(face.centerZ);

		features.push_back(face.normalX);
		features.push_back(face.normalY);
		features.push_back(face.normalZ);

		features.push_back(face.area);

		features.push_back(face.edgeLength0);
		features.push_back(face.edgeLength1);
		features.push_back(face.edgeLength2);

		features.push_back(face.angle0);
		features.push_back(face.angle1);
		features.push_back(face.angle2);
	}

	auto tensor = torch::from_blob(
		features.data(),
		{
			static_cast<int64_t>(mesh.faces.size()),
			13
		},
		torch::kFloat32
	).clone();

	//---------------------------------------------------------
	// featureごとに正規化
	//---------------------------------------------------------
	auto mean = tensor.mean(0, true);
	auto std = tensor.std(0, true);

	tensor = (tensor - mean) / (std + 1e-6f);
	return tensor;
}

void MeshSegmentationAI::Test()
{
	auto trainData = LoadTrainData(
		"E:\\cgModel\\MeshsegBenchmark-1.0\\data\\meshcnn",
		"E:\\cgModel\\MeshsegBenchmark-1.0\\data\\seg\\Benchmark");
	Train(trainData, "E:\\cgModel\\MeshsegBenchmark-1.0\\data\\meshcnn\\learn.pt");
	Predict(
		"E:\\cgModel\\MeshsegBenchmark-1.0\\data\\predict\\bunny.meshcnn",
		"E:\\cgModel\\MeshsegBenchmark-1.0\\data\\predict\\bunny.seg");
}

std::vector<MeshSegmentationAI::TrainData> MeshSegmentationAI::LoadTrainData(const std::filesystem::path& meshRoot, const std::filesystem::path& segRoot)
{
	namespace fs = std::filesystem;
	std::map<std::string, TrainData> dataMap;
	if (!fs::exists(meshRoot) || !fs::exists(segRoot)) { return {}; }
	for (const auto& entry : fs::recursive_directory_iterator(segRoot)) {
		if (!entry.is_regular_file()) { continue; }
		const auto& segPath = entry.path();
		if (segPath.extension() != ".seg") { continue; }

		//-----------------------------------------------------
		// 1_0.seg
		// 1_1.seg
		// ↓
		// meshName = "1"
		//-----------------------------------------------------
		const std::string stem = segPath.stem().string();
		const auto separator = stem.find_last_of('_');
		if (separator == std::string::npos) { continue; }
		const std::string meshName = stem.substr(0, separator);

		//-----------------------------------------------------
		// meshcnn/1.meshcnn
		//-----------------------------------------------------
		const fs::path meshPath = meshRoot / (meshName + ".meshcnn");

		if (!fs::exists(meshPath)) {
			std::cout << "Mesh file not found : " << meshPath << std::endl;
			continue;
		}

		//-----------------------------------------------------
		// Category
		//
		// Benchmark/1/1_0.seg
		//           ↑
		//-----------------------------------------------------
		const std::string category = segPath.parent_path().filename().string();

		//-----------------------------------------------------
		// Category + MeshNameをKeyにする
		//-----------------------------------------------------
		const std::string key = category + "/" + meshName;

		auto it = dataMap.find(key);

		if (it == dataMap.end()) {
			TrainData data;
			data.meshPath = meshPath;
			data.meshData = LoadMeshData(meshPath.string());
			data.features = CreateFeatureTensor(data.meshData);
			data.neighbors = CreateNeighborTensor(data.meshData);

			data.category = category;
			it = dataMap.emplace(key, std::move(data)).first;
		}

		SegData seg;
		seg.labels = LoadSeg(segPath.string());
		seg.path = segPath;
		it->second.segs.push_back(seg);
	}

	//---------------------------------------------------------
	// map -> vector
	//---------------------------------------------------------
	std::vector<TrainData> result;
	result.reserve(dataMap.size());

	for (auto& [key, data] : dataMap) {
		std::sort(data.segs.begin(), data.segs.end(),
			[](const SegData& a, const SegData& b)
		{
			return a.path < b.path;
		});
		result.push_back(std::move(data));
	}

	return result;
}

torch::Tensor MeshSegmentationAI::CreateBoundaryTarget(const MeshSegmentationAI::TrainData& trainData)
{
	const auto& mesh = trainData.meshData;
	std::vector<float> targets(mesh.faces.size() * 3, 0.0f);
	if (trainData.segs.empty()) { return {}; }

	for (const auto& seg : trainData.segs) {
		if (seg.labels.size() != mesh.faces.size()) {
			continue;
		}

		for (size_t i = 0; i < mesh.faces.size(); ++i) {
			const auto& face = mesh.faces[i];
			const int32_t neighbors[3] =
			{
				face.neighbor0,
				face.neighbor1,
				face.neighbor2
			};

			for (int j = 0; j < 3; ++j) {
				const auto neighbor = neighbors[j];
				if (neighbor < 0 || neighbor >= mesh.faces.size()) {
					continue;
				}
				if (seg.labels[i] != seg.labels[neighbor]) {
					targets[i * 3 + j] += 1.0f;
				}
			}
		}
	}

	const float invSegCount = 1.0f / static_cast<float>(trainData.segs.size());

	for (auto& value : targets) { value *= invSegCount; }

	return torch::from_blob(targets.data(), { static_cast<int64_t>(mesh.faces.size()), 3 }, torch::kFloat32).clone();
}
void MeshSegmentationAI::Train(const std::vector<TrainData>& trainDatas, const std::filesystem::path& modelPath)
{
	if (trainDatas.empty()) { return; }

	//---------------------------------------------------------
	// Model
	//---------------------------------------------------------
	m_model = SegmentationNet();

	//---------------------------------------------------------------------
	// 学習済みモデルが存在するなら読み込んで終了
	//---------------------------------------------------------------------
	if (std::filesystem::exists(modelPath)) {

		std::cout << "Load By File : " << modelPath << std::endl;

		torch::load(m_model, modelPath.string());

		m_model->eval();

		return;
	}

	m_model->train();

	torch::optim::Adam optimizer(m_model->parameters(), torch::optim::AdamOptions(0.001));

	//---------------------------------------------------------
	// Targetを事前生成
	//---------------------------------------------------------
	std::vector<torch::Tensor> targets;
	targets.reserve(trainDatas.size());

	for (const auto& trainData : trainDatas) {
		targets.push_back(CreateBoundaryTarget(trainData));
	}

	float positiveCount = 0.0f;
	float negativeCount = 0.0f;

	for (const auto& target : targets) {
		if (!target.defined() || target.numel() == 0) {
			continue;
		}

		const float positive = target.sum().item<float>();
		const float total = static_cast<float>(target.numel());
		positiveCount += positive;
		negativeCount += total - positive;
	}

	auto positiveWeight = std::sqrt(	negativeCount /	std::max(positiveCount, 1.0f));
	std::cout << "Positive Weight : " << positiveWeight << std::endl;
	auto posWeight = torch::tensor({ positiveWeight }, torch::kFloat32);
	auto options = torch::nn::functional::BinaryCrossEntropyWithLogitsFuncOptions().pos_weight(posWeight);
	//---------------------------------------------------------
	// Train
	//---------------------------------------------------------
	const int epochNum = 300;
	for (int epoch = 0; epoch < epochNum; ++epoch) {

		double totalLoss = 0.0;
		int sampleCount = 0;

		//---------------------------------------------------------
		// Epoch Metrics
		//---------------------------------------------------------
		int64_t totalTP = 0;
		int64_t totalFP = 0;
		int64_t totalFN = 0;
		int64_t totalTN = 0;

		double probabilitySum = 0.0;
		int64_t probabilityCount = 0;

		float probabilityMin =
			std::numeric_limits<float>::max();

		float probabilityMax =
			std::numeric_limits<float>::lowest();

		int64_t boundaryCount = 0;
		int64_t elementCount = 0;

		for (size_t i = 0; i < trainDatas.size(); ++i) {

			const auto& trainData = trainDatas[i];

			if (trainData.meshData.faces.empty()) {
				continue;
			}

			if (!targets[i].defined() ||
				targets[i].numel() == 0) {
				continue;
			}

			//-----------------------------------------------------
			// Forward
			//-----------------------------------------------------
			auto outputs = m_model->forward(
				trainData.features,
				trainData.neighbors);

			//-----------------------------------------------------
			// Loss
			//-----------------------------------------------------
			auto options =
				torch::nn::functional::
				BinaryCrossEntropyWithLogitsFuncOptions()
				.pos_weight(posWeight);

			auto loss =
				torch::nn::functional::
				binary_cross_entropy_with_logits(
					outputs,
					targets[i],
					options);

			//-----------------------------------------------------
			// Backward
			//-----------------------------------------------------
			optimizer.zero_grad();
			loss.backward();
			optimizer.step();

			totalLoss += loss.item<double>();
			++sampleCount;

			//-----------------------------------------------------
			// Probability
			//-----------------------------------------------------
			auto probability =
				torch::sigmoid(outputs.detach());

			probabilityMin = std::min(
				probabilityMin,
				probability.min().item<float>());

			probabilityMax = std::max(
				probabilityMax,
				probability.max().item<float>());

			probabilitySum +=
				probability.sum().item<double>();

			probabilityCount +=
				probability.numel();

			//-----------------------------------------------------
			// Binary Evaluation
			//-----------------------------------------------------
			constexpr float threshold = 0.2f;

			auto predicted =
				probability >= threshold;

			auto actual =
				targets[i] >= threshold;

			const int64_t tp =
				(predicted & actual)
				.sum()
				.item<int64_t>();

			const int64_t fp =
				(predicted & ~actual)
				.sum()
				.item<int64_t>();

			const int64_t fn =
				(~predicted & actual)
				.sum()
				.item<int64_t>();

			const int64_t tn =
				(~predicted & ~actual)
				.sum()
				.item<int64_t>();

			totalTP += tp;
			totalFP += fp;
			totalFN += fn;
			totalTN += tn;

			boundaryCount +=
				actual.sum().item<int64_t>();

			elementCount +=
				actual.numel();
		}

		if (sampleCount == 0) {
			continue;
		}

		//---------------------------------------------------------
		// Epoch Metrics
		//---------------------------------------------------------
		const double precision =
			static_cast<double>(totalTP) /
			(static_cast<double>(totalTP + totalFP) + 1e-8);

		const double recall =
			static_cast<double>(totalTP) /
			(static_cast<double>(totalTP + totalFN) + 1e-8);

		const double f1 =
			2.0 * precision * recall /
			(precision + recall + 1e-8);

		const double accuracy =
			static_cast<double>(totalTP + totalTN) /
			(static_cast<double>(
				totalTP + totalTN + totalFP + totalFN) + 1e-8);

		const double boundaryRate =
			static_cast<double>(boundaryCount) /
			(static_cast<double>(elementCount) + 1e-8);

		const double probabilityMean =
			probabilitySum /
			(static_cast<double>(probabilityCount) + 1e-8);

		//---------------------------------------------------------
		// Log
		//---------------------------------------------------------
		if (epoch % 10 == 0) {

			std::cout
				<< "Epoch: " << epoch
				<< ", Loss: "
				<< totalLoss / sampleCount
				<< ", Accuracy: "
				<< accuracy * 100.0
				<< "%"
				<< ", Precision: "
				<< precision * 100.0
				<< "%"
				<< ", Recall: "
				<< recall * 100.0
				<< "%"
				<< ", F1: "
				<< f1 * 100.0
				<< "%"
				<< ", BoundaryRate: "
				<< boundaryRate * 100.0
				<< "%"
				<< ", Prob[min,max,mean]: "
				<< probabilityMin
				<< ", "
				<< probabilityMax
				<< ", "
				<< probabilityMean
				<< std::endl;
		}
	}
	//for (int epoch = 0; epoch < epochNum; ++epoch) {

	//	float totalLoss = 0.0f;
	//	float totalAccuracy = 0.0f;
	//	int sampleCount = 0;

	//	for (size_t i = 0; i < trainDatas.size(); ++i) {
	//		const auto& trainData = trainDatas[i];

	//		if (trainData.meshData.faces.empty()) {
	//			continue;
	//		}

	//		if (!targets[i].defined() || targets[i].numel() == 0) {
	//			continue;
	//		}

	//		//-------------------------------------------------
	//		// Forward
	//		//
	//		// outputs : [FaceNum, 3]
	//		//-------------------------------------------------
	//		auto outputs = m_model->forward(trainData.features, trainData.neighbors);

	//		//-------------------------------------------------
	//		// Boundary Prediction
	//		//
	//		// target = 0.0 ～ 1.0
	//		//-------------------------------------------------
	//		auto loss = torch::nn::functional::binary_cross_entropy_with_logits(outputs, targets[i], options);

	//		//-------------------------------------------------
	//		// Backward
	//		//-------------------------------------------------
	//		optimizer.zero_grad();
	//		loss.backward();
	//		optimizer.step();

	//		//-------------------------------------------------
	//		// Accuracy
	//		//-------------------------------------------------
	//		auto probability = torch::sigmoid(outputs);
	//		auto predicted = probability >= 0.1f;
	//		auto targetBoundary = targets[i] >= 0.1f;
	//		const float accuracy = predicted.eq(targetBoundary).to(torch::kFloat32).mean().item<float>();
	//		totalLoss += loss.item<float>();
	//		totalAccuracy += accuracy;
	//		++sampleCount;
	//	}

	//	if (sampleCount == 0) {
	//		continue;
	//	}

	//	if (epoch % 10 == 0) {

	//		std::cout
	//			<< "Epoch: "
	//			<< epoch
	//			<< ", Loss: "
	//			<< totalLoss / sampleCount
	//			<< ", Accuracy: "
	//			<< totalAccuracy /
	//			sampleCount * 100.0f
	//			<< "%"
	//			<< std::endl;
	//	}
	//}

	//---------------------------------------------------------
	// Save
	//---------------------------------------------------------
	std::cout << "Save File : " << modelPath << std::endl;

	torch::save(m_model, modelPath.string());
}

std::vector<int32_t> MeshSegmentationAI::Predict(const std::string& meshPath)
{
	std::vector<int32_t> result;
	if (!m_model) { return result; }
	auto mesh = LoadMeshData(meshPath);
	if (mesh.faces.empty()) { return result; }

	torch::NoGradGuard noGrad;

	m_model->eval();

	auto features = CreateFeatureTensor(mesh);
	auto neighbors = CreateNeighborTensor(mesh);
	auto outputs = m_model->forward(features, neighbors);
	auto predicted = outputs.argmax(1);
	predicted = predicted.to(torch::kCPU).to(torch::kInt32).contiguous();
	result.resize(predicted.numel());

	std::memcpy(result.data(), predicted.data_ptr<int32_t>(), result.size() * sizeof(int32_t));

	return result;
}

void MeshSegmentationAI::Predict(const std::string& meshPath, const std::string& outPath)
{
	auto labels = Predict(meshPath);
	if (labels.empty()) { return; }
	std::ofstream ofs(outPath, std::ios::binary);
	if (!ofs.is_open()) { return; }
	const uint32_t faceCount = static_cast<uint32_t>(labels.size());
	ofs.write(reinterpret_cast<const char*>(&faceCount), sizeof(faceCount));
	ofs.write(reinterpret_cast<const char*>(labels.data()), sizeof(int32_t) * labels.size());
}
}

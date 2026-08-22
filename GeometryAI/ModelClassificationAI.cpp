#include "ModelClassificationAI.h"
#include "TorchUtility.h"
#include "Utility.h"
#include <torch/torch.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <random>
namespace KI
{

ModelClassificationAI::ModelClassificationAI()
	:m_device(torch::cuda::is_available() ? torch::kCUDA : torch::kCPU)
{

}
ModelClassificationAI::~ModelClassificationAI()
{

}


bool ModelClassificationAI::Execute(const std::string& command)
{
	auto parsed = Parse(command);
	if (parsed.size() == 0) return false;
	if (parsed[0] != GetCommandName()) { return false; }
	const std::string& action = parsed[1];
	if (action == "--train") {
		Train("E:\\cgModel\\ModelNet40");
		return true;
	} else if (action == "--predict") {
		Predict(parsed[2], parsed[3]);
		return true;
	}

	return false;
}

void ModelClassificationAI::Train(const std::string& path)
{
	// 学習済みモデルのパス
	// _点数_epoch数
	namespace fs = std::filesystem;
	fs::path modelPath(path + "/train_2048_30.pt");
	//---------------------------------------------------------------------
	// 学習済みモデルが存在するなら読み込んで終了
	//---------------------------------------------------------------------
	if (fs::exists(modelPath)) {
		std::cout << "Load By File : " << modelPath << std::endl;
		torch::load(m_model, modelPath.string());
		return;
	}

	struct PointCloudSample
	{
		std::filesystem::path filePath;
		int64_t label;
	};

	std::vector<std::filesystem::path> categoryDirectories;

	std::filesystem::path trainPath(path);
	int fileNum = 0;
	// airplane、bathtubなどのカテゴリフォルダを取得
	for (const auto& entry : std::filesystem::directory_iterator(trainPath)) {
		if (!entry.is_directory()) {continue; }

		const auto trainDirectory =	entry.path() / "train_2048";

		if (std::filesystem::is_directory(trainDirectory)) {
			categoryDirectories.push_back(entry.path());
		}
	}

	// ラベルが実行ごとに変わらないようにソート
	std::sort(categoryDirectories.begin(), categoryDirectories.end());


	std::vector<std::string> classNames;
	std::vector<PointCloudSample> samples;

	for (size_t label = 0; label < categoryDirectories.size(); ++label) {
		const auto& categoryDirectory = categoryDirectories[label];
		const std::string className = categoryDirectory.filename().string();
		classNames.push_back(categoryDirectory.filename().string());
		const auto trainDirectory = categoryDirectory / "train_2048";
		for (const auto& entry : std::filesystem::recursive_directory_iterator(trainDirectory)) {
			if (!entry.is_regular_file()) { continue; }
			samples.push_back({ entry.path(), static_cast<int64_t>(label) });
		}

		std::cout << label << " : " << className << std::endl;
	}

	if (samples.empty()) {
		throw std::runtime_error("Training data was not found.");
	}


	m_model = ModelClassification( static_cast<int64_t>(classNames.size()));
	m_model->to(m_device);

	torch::optim::Adam optimizer(m_model->parameters(), torch::optim::AdamOptions(0.001));

	std::mt19937 randomEngine(std::random_device{}());

	for (int epoch = 0; epoch < 30; ++epoch) {
		m_model->train();

		std::shuffle(samples.begin(), samples.end(), randomEngine);

		float totalLoss = 0.0f;
		float correctCount = 0;
		float totalCount = 0;
		float processedBatchCount = 0;

		size_t batchSize = 32;
		for (size_t batchBegin = 0; batchBegin < samples.size(); batchBegin += batchSize) {
			const size_t batchEnd = std::min(batchBegin + batchSize, samples.size());

			std::vector<torch::Tensor> pointTensors;
			std::vector<int64_t> labelValues;

			pointTensors.reserve(batchEnd - batchBegin);
			labelValues.reserve(batchEnd - batchBegin);

			for (size_t i = batchBegin; i < batchEnd; ++i) {
				auto pointData = GeometryUtility::LoadPointCloud(samples[i].filePath.string());
				pointData = GeometryUtility::NormalizePointCloud(pointData);
				auto pointTensor = TorchUtility::ToTensor1D(pointData, 3);
				pointTensors.push_back(std::move(pointTensor));
				labelValues.push_back(samples[i].label);
			}

			/* points: [B, 2048, 3] */
			torch::Tensor points = torch::stack(pointTensors).to(m_device);
			/* labels: [B] */
			torch::Tensor labels = torch::tensor(labelValues, torch::TensorOptions().dtype(torch::kInt64)).to(m_device);

			optimizer.zero_grad();
			torch::Tensor logits = m_model->forward(points);
			torch::Tensor loss = torch::nn::functional::cross_entropy(logits, labels);

			loss.backward();
			optimizer.step();

			totalLoss += loss.item<float>();
			correctCount += logits.argmax(1).eq(labels).sum().item<int64_t>();
			totalCount += labels.size(0);
			++processedBatchCount;

			const float progress =	static_cast<float>(batchEnd) /	static_cast<float>(samples.size());
			std::cout << "Progress: " << progress * 100.0f << "% (" << batchEnd << " / " << samples.size() << ")\n";
		}

		const float averageLoss = totalLoss / processedBatchCount;
		const float accuracy = correctCount / totalCount;
		std::cout << "Epoch: " << epoch << ", Loss: " << averageLoss << ", Accuracy: " << accuracy * 100.0f << "%" << std::endl;
	}

	//---------------------------------------------------------------------
	// 保存
	//---------------------------------------------------------------------
	std::cout << "Save File : " << modelPath << std::endl;
	torch::save(m_model, modelPath.string());
}

void ModelClassificationAI::Test()
{
	namespace fs = std::filesystem;

	fs::path modelNetPath(R"(E:\cgModel\ModelNet40)");

	//---------------------------------------------------------------------
	// モデル読み込み
	//---------------------------------------------------------------------
	Train(modelNetPath.string());

	m_model->eval();
	torch::NoGradGuard noGrad;

	//---------------------------------------------------------------------
	// カテゴリフォルダ取得
	//---------------------------------------------------------------------
	std::vector<fs::path> categoryDirectories;

	for (const auto& entry : fs::directory_iterator(modelNetPath)) {
		if (!entry.is_directory()) continue;

		const auto testDirectory = entry.path() / "test_2048";
		if (fs::is_directory(testDirectory)) {
			categoryDirectories.push_back(entry.path());
		}
	}

	// Train時と同じラベル順序にする
	std::sort(categoryDirectories.begin(), categoryDirectories.end());

	const size_t classCount = categoryDirectories.size();

	std::vector<std::string> classNames;
	classNames.reserve(classCount);

	for (const auto& categoryDirectory : categoryDirectories) {
		classNames.push_back(categoryDirectory.filename().string());
	}

	//---------------------------------------------------------------------
	// confusionMatrix[正解ラベル][予測ラベル]
	//---------------------------------------------------------------------
	std::vector<std::vector<int64_t>> confusionMatrix(
		classCount, std::vector<int64_t>(classCount, 0));

	int64_t totalCorrectCount = 0;
	int64_t totalSampleCount = 0;

	//---------------------------------------------------------------------
	// 各カテゴリをテスト
	//---------------------------------------------------------------------
	for (size_t label = 0; label < classCount; ++label) {
		const auto& categoryDirectory = categoryDirectories[label];
		const auto testDirectory = categoryDirectory / "test_2048";

		int64_t categoryCorrectCount = 0;
		int64_t categorySampleCount = 0;

		for (const auto& entry : fs::recursive_directory_iterator(testDirectory)) {
			if (!entry.is_regular_file()) continue;

			auto pointData =
				GeometryUtility::LoadPointCloud(entry.path().string());
			pointData = GeometryUtility::NormalizePointCloud(pointData);

			auto pointTensor = TorchUtility::ToTensor1D(pointData, 3);
			torch::Tensor points = pointTensor.unsqueeze(0).to(m_device);

			torch::Tensor logits = m_model->forward(points);
			const int64_t predictedLabel = logits.argmax(1).item<int64_t>();

			++confusionMatrix[label][predictedLabel];

			if (predictedLabel == static_cast<int64_t>(label)) {
				++categoryCorrectCount;
				++totalCorrectCount;
			}

			++categorySampleCount;
			++totalSampleCount;
		}

		const float accuracy = categorySampleCount > 0
			? static_cast<float>(categoryCorrectCount) / categorySampleCount
			: 0.0f;

		std::cout << label << " : " << classNames[label]
			<< " Accuracy: " << accuracy * 100.0f << "% ("
			<< categoryCorrectCount << " / " << categorySampleCount << ")"
			<< std::endl;
	}

	//---------------------------------------------------------------------
	// 全体精度
	//---------------------------------------------------------------------
	const float totalAccuracy = totalSampleCount > 0
		? static_cast<float>(totalCorrectCount) / totalSampleCount
		: 0.0f;

	std::cout << "----------------------------------------" << std::endl;
	std::cout << "Total Accuracy: " << totalAccuracy * 100.0f << "% ("
		<< totalCorrectCount << " / " << totalSampleCount << ")" << std::endl;

	//---------------------------------------------------------------------
	// 誤分類結果
	//---------------------------------------------------------------------
	std::cout << std::endl;
	std::cout << "========== Misclassification ==========" << std::endl;

	for (size_t actual = 0; actual < classCount; ++actual) {
		int64_t incorrectCount = 0;

		for (size_t predicted = 0; predicted < classCount; ++predicted) {
			if (actual == predicted) continue;
			incorrectCount += confusionMatrix[actual][predicted];
		}

		if (incorrectCount == 0) continue;

		std::cout << std::endl;
		std::cout << actual << " : " << classNames[actual]
			<< " (" << incorrectCount << " incorrect)" << std::endl;

		for (size_t predicted = 0; predicted < classCount; ++predicted) {
			if (actual == predicted) continue;

			const int64_t count = confusionMatrix[actual][predicted];
			if (count == 0) continue;

			std::cout << "  -> " << predicted << " : " << classNames[predicted]
				<< " = " << count << std::endl;
		}
	}
}
//void ModelClassificationAI::Test()
//{
//	namespace fs = std::filesystem;
//
//	fs::path modelNetPath(R"(E:\cgModel\ModelNet40)");
//
//	//---------------------------------------------------------------------
//	// モデル読み込み
//	//---------------------------------------------------------------------
//	Train(modelNetPath.string());
//
//	m_model->eval();
//	torch::NoGradGuard noGrad;
//
//	//---------------------------------------------------------------------
//	// カテゴリフォルダ取得
//	//---------------------------------------------------------------------
//	std::vector<fs::path> categoryDirectories;
//	for (const auto& entry : fs::directory_iterator(modelNetPath)) {
//		if (!entry.is_directory()) {
//			continue;
//		}
//
//		const auto testDirectory = entry.path() / "test_2048";
//
//		if (fs::is_directory(testDirectory)) {
//			categoryDirectories.push_back(entry.path());
//		}
//	}
//
//	//---------------------------------------------------------------------
//	// Train時と同じラベル順序にする
//	//---------------------------------------------------------------------
//	std::sort(categoryDirectories.begin(), categoryDirectories.end());
//
//	float totalCorrectCount = 0;
//	float totalSampleCount = 0;
//
//	//---------------------------------------------------------------------
//	// 各カテゴリをテスト
//	//---------------------------------------------------------------------
//	for (size_t label = 0; label < categoryDirectories.size(); ++label) {
//
//		const auto& categoryDirectory = categoryDirectories[label];
//		const auto testDirectory = categoryDirectory / "test_2048";
//		const std::string className = categoryDirectory.filename().string();
//
//		float categoryCorrectCount = 0;
//		float categorySampleCount = 0;
//
//		for (const auto& entry :
//			fs::recursive_directory_iterator(testDirectory)) {
//
//			if (!entry.is_regular_file()) { continue; }
//
//			//-----------------------------------------------------------------
//			// 点群読み込み
//			//-----------------------------------------------------------------
//			auto pointData = GeometryUtility::LoadPointCloud(entry.path().string());
//
//			pointData = GeometryUtility::NormalizePointCloud(pointData);
//			auto pointTensor = TorchUtility::ToTensor1D(pointData, 3);
//
//			//-----------------------------------------------------------------
//			// [2048, 3]
//			//        ↓
//			// [1, 2048, 3]
//			//-----------------------------------------------------------------
//			torch::Tensor points = pointTensor.unsqueeze(0).to(m_device);
//
//			//-----------------------------------------------------------------
//			// 推論
//			//-----------------------------------------------------------------
//			torch::Tensor logits = m_model->forward(points);
//
//			const int64_t predictedLabel = logits.argmax(1).item<int64_t>();
//
//			//-----------------------------------------------------------------
//			// 正解判定
//			//-----------------------------------------------------------------
//			if (predictedLabel == static_cast<int64_t>(label)) {
//				++categoryCorrectCount;
//				++totalCorrectCount;
//			}
//
//			++categorySampleCount;
//			++totalSampleCount;
//		}
//
//		//---------------------------------------------------------------------
//		// カテゴリごとの精度
//		//---------------------------------------------------------------------
//		const float accuracy = categorySampleCount > 0 ? categoryCorrectCount / categorySampleCount : 0.0f;
//
//		std::cout
//			<< label << " : " << className << " Accuracy: "
//			<< accuracy * 100.0f << "% (" << categoryCorrectCount
//			<< " / " << categorySampleCount << ")"
//			<< std::endl;
//	}
//
//	//---------------------------------------------------------------------
//	// 全体精度
//	//---------------------------------------------------------------------
//	const float totalAccuracy = totalSampleCount > 0 ? totalCorrectCount / totalSampleCount : 0.0f;
//
//	std::cout << "----------------------------------------" << std::endl;
//
//	std::cout << "Total Accuracy: " << totalAccuracy * 100.0f
//		<< "% (" << totalCorrectCount << " / " << totalSampleCount << ")" << std::endl;
//}

void ModelClassificationAI::Predict(const std::string& in, const std::string& out)
{
	auto result = Predict(GeometryUtility::LoadPointCloud(in));

	// write binary float array
	std::ofstream ofs(out, std::ios::binary);
    if (!ofs) {
        std::cout << "Failed to open output file: " << out << std::endl;
        return;
    }
    ofs.write(reinterpret_cast<const char*>(result), sizeof(int));
}

int ModelClassificationAI::Predict(const std::vector<float>& position)
{
	constexpr size_t pointCount = 2048;
	constexpr size_t componentCount = 3;
	if (position.size() != pointCount * componentCount) { return -1; }

	torch::NoGradGuard noGrad;
	m_model->eval();

	auto normalized = GeometryUtility::NormalizePointCloud(position);
	auto input = TorchUtility::ToTensor1D(normalized, 3);
	// [2048 * 3] → [1, 2048, 3]
	input = input.view({ 1, static_cast<int64_t>(pointCount), 3 }).to(m_device);

	// output: [1, クラス数]
	auto output = m_model->forward(input);
	// predicted: [1]
	return output.argmax(1).item<int>();
}

}
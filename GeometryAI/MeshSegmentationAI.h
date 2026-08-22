#ifndef MESH_SEGMENTATION_AI_H
#define MESH_SEGMENTATION_AI_H
#include "ICommandReceiver.h"
#include <string>
#include <filesystem>
#include <torch/torch.h>
#include <tuple>
namespace KI
{

class MeshSegmentationAI : public ICommandReceiver
{
private:



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

	struct MeshData
	{
		std::vector<FaceSerializer> faces;
	};

	struct SegData
	{
		std::filesystem::path path;
		std::vector<int64_t> labels;
	};

	struct TrainData
	{
		std::filesystem::path meshPath;
		MeshData meshData;
		torch::Tensor features;
		torch::Tensor neighbors;
		std::vector<SegData> segs;

		std::string category;
	};

	struct 	SegmentationNetImpl : torch::nn::Module
	{
		torch::nn::Linear input{ nullptr };
		torch::nn::Linear conv0{ nullptr };
		torch::nn::Linear conv1{ nullptr };
		torch::nn::Linear conv2{ nullptr };
		torch::nn::Linear output{ nullptr };

		SegmentationNetImpl()
		{
			input = register_module("input", torch::nn::Linear(13, 64));
			conv0 = register_module("conv0", torch::nn::Linear(128, 64));
			conv1 = register_module("conv1", torch::nn::Linear(128, 64));
			conv2 = register_module("conv2", torch::nn::Linear(128, 64));

			//-----------------------------------------------------
			// neighbor0 / neighbor1 / neighbor2
			// ‚»‚ê‚¼‚ê‚Æ‚Ì‹«ŠElogit
			//-----------------------------------------------------
			output = register_module("output", torch::nn::Linear(64, 3));
		}

		torch::Tensor Aggregate(const torch::Tensor& x, const torch::Tensor& neighbors)
		{
			const auto faceNum = x.size(0);
			const auto featureNum = x.size(1);

			auto index = neighbors.reshape({ -1 });
			auto neighborFeature = x.index_select(0, index);

			neighborFeature = neighborFeature.reshape({ faceNum, 3, featureNum });

			return neighborFeature.mean(1);
		}

		torch::Tensor forward(torch::Tensor x, const torch::Tensor& neighbors)
		{
			x = torch::relu(input(x));

			auto n0 = Aggregate(x, neighbors);
			x = torch::relu(conv0(torch::cat({ x, n0 }, 1)));

			auto n1 = Aggregate(x, neighbors);
			x = torch::relu(conv1(torch::cat({ x, n1 }, 1)));

			auto n2 = Aggregate(x, neighbors);
			x = torch::relu(conv2(torch::cat({ x, n2 }, 1)));

			return output(x);
		}
	};

	TORCH_MODULE(SegmentationNet);

public:

	MeshSegmentationAI() {}
	~MeshSegmentationAI() {}

	virtual std::string GetCommandName() const
	{
		return std::string("MeshSegmentationAI");
	}

	virtual bool Execute(const std::string& command) override { return false; };
	void Test();
private:
	torch::Tensor CreateBoundaryTarget(const TrainData& trainData);
	std::vector<MeshSegmentationAI::TrainData> LoadTrainData(const std::filesystem::path& meshRoot, const std::filesystem::path& segRoot);

	MeshData LoadMeshData(const std::string& path);
	std::vector<int64_t> LoadSeg(const std::string& path);

	torch::Tensor CreateFeatureTensor(const MeshData& mesh);
	torch::Tensor CreateNeighborTensor(const MeshData& mesh);

	void Train(const std::vector<MeshSegmentationAI::TrainData>& trainDatas, const std::filesystem::path& segPath);

	std::vector<int32_t> Predict(const std::string& meshPath);
	void Predict(const std::string& meshPath, const std::string& outPath);


	SegmentationNet m_model{ nullptr };
};


}

#endif MESH_SEGMENTATION_AI_H
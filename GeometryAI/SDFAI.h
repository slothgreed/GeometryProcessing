#ifndef SDF_AI_H
#define SDF_AI_H
#include "ICommandReceiver.h"
#include <string>
#include <torch/torch.h>

namespace KI
{

class SDFAI : public ICommandReceiver
{
private:

    struct NeuralSDFImpl : torch::nn::Module
    {
        torch::nn::Sequential net;
        NeuralSDFImpl()
        {
            net = torch::nn::Sequential(
                torch::nn::Linear(3, 64),
                torch::nn::ReLU(),
                torch::nn::Linear(64, 64),
                torch::nn::ReLU(),
                torch::nn::Linear(64, 64),
                torch::nn::ReLU(),
                torch::nn::Linear(64, 1)
            );
            register_module("net", net);
        }

        torch::Tensor forward(torch::Tensor x)
        {
            return net->forward(x);
        }

    };
    TORCH_MODULE(NeuralSDF);

    struct VoxelData
    {
        float min[3];
        float max[3];

        int resolution[3];

        std::vector<float> data;
        std::vector<float> CreatePosition() const;
    };

public:
	SDFAI() {};
	~SDFAI() {};
    virtual std::string GetCommandName() const { return std::string("SDFAI"); }
	virtual bool Execute(const std::string& command) override;
private:
    // voxel file
    void Train(const std::string& path);
	// in : position file (ex x, y, z, x, y, z, ...)
	// out : sdf file (ex float, float, float, ...)
    void Predict(const std::string& in, const std::string& out);
    std::vector<float> Predict(const std::vector<float>& position);
    VoxelData Load(const std::string& path);

    NeuralSDF m_model;

};

}


#endif SDF_AI_H
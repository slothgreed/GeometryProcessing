#ifndef DIFFUSION_AI_H
#define DIFFUSION_AI_H

#include "ICommandReceiver.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <torch/torch.h>

namespace KI
{

class DiffusionAI : public ICommandReceiver
{
private:
    struct DiffusionModelImpl : torch::nn::Module
    {
        DiffusionModelImpl();

        torch::Tensor forward(const torch::Tensor& points,
                              const torch::Tensor& timestep);
        void SetPointCount(int64_t pointCount);
        int64_t GetPointCount() const;

        torch::nn::Sequential pointEncoder{ nullptr };
        torch::nn::Sequential noiseDecoder{ nullptr };
        torch::Tensor pointCountBuffer;
    };
    TORCH_MODULE(DiffusionModel);

public:
    DiffusionAI() = default;
    ~DiffusionAI() = default;

    std::string GetCommandName() const override
    {
        return std::string("DiffusionAI");
    }

    bool Execute(const std::string& command) override;

private:
    static constexpr int64_t DiffusionSteps = 200;
    static constexpr int64_t EpochCount = 500;

    void Train(const std::string& path);
    void Predict(const std::string& modelPath, const std::string& outPath);

    torch::Tensor LoadPointCloud(const std::filesystem::path& path) const;
    void SavePointCloud(const std::filesystem::path& path,
                        const torch::Tensor& points) const;
    torch::Device GetDevice() const;

    DiffusionModel m_model;
    torch::Tensor m_predictPoint;
    int64_t m_predictStep = -1;
    bool m_modelReady = false;
};

}

#endif // DIFFUSION_AI_H

#include "DiffusionAI.h"

#include "TorchUtility.h"
#include "Utility.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace KI
{

DiffusionAI::DiffusionModelImpl::DiffusionModelImpl()
{
    // XYZ and a normalized diffusion timestep are encoded per point.
    pointEncoder = register_module("point_encoder", torch::nn::Sequential(
        torch::nn::Linear(4, 128),
        torch::nn::SiLU(),
        torch::nn::Linear(128, 128),
        torch::nn::SiLU()));

    // Local point features are combined with a PointNet-style global feature.
    noiseDecoder = register_module("noise_decoder", torch::nn::Sequential(
        torch::nn::Linear(256, 128),
        torch::nn::SiLU(),
        torch::nn::Linear(128, 64),
        torch::nn::SiLU(),
        torch::nn::Linear(64, 3)));

    pointCountBuffer = register_buffer(
        "point_count", torch::zeros({ 1 }, torch::kInt64));
}

torch::Tensor DiffusionAI::DiffusionModelImpl::forward(const torch::Tensor& points, const torch::Tensor& timestep)
{
    const int64_t batchSize = points.size(0);
    const int64_t pointCount = points.size(1);
    auto timeFeature = timestep.reshape({ batchSize, 1, 1 })
        .expand({ batchSize, pointCount, 1 });
    auto localFeature = pointEncoder->forward(
        torch::cat({ points, timeFeature }, 2));
    auto globalFeature = std::get<0>(localFeature.max(1, true))
        .expand({ batchSize, pointCount, localFeature.size(2) });
    return noiseDecoder->forward(
        torch::cat({ localFeature, globalFeature }, 2));
}

void DiffusionAI::DiffusionModelImpl::SetPointCount(int64_t pointCount)
{
    pointCountBuffer.fill_(pointCount);
}

int64_t DiffusionAI::DiffusionModelImpl::GetPointCount() const
{
    return pointCountBuffer.item<int64_t>();
}

bool DiffusionAI::Execute(const std::string& command)
{
    const auto parsed = ICommandReceiver::Parse(command);
    if (parsed.empty() || parsed[0] != GetCommandName()) {
        return false;
    }

    try {
        if (parsed.size() >= 3 && parsed[1] == "--train") {
            Train(parsed[2]);
            return true;
        }
        if (parsed.size() >= 4 && parsed[1] == "--predict") {
            Predict(parsed[2], parsed[3]);
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "DiffusionAI error: " << e.what() << std::endl;
        return false;
    }

    std::cout
        << "Usage: " << GetCommandName() << " --train <point cloud file>\n"
        << "   or: " << GetCommandName()
        << " --predict <model.pt> <output.ki_bin>" << std::endl;
    return false;
}

torch::Tensor DiffusionAI::LoadPointCloud(const std::filesystem::path& path) const
{
    auto points = GeometryUtility::LoadPointCloud(path.string());
    points = GeometryUtility::NormalizePointCloud(points);
    if (points.empty()) {
        throw std::runtime_error("Point cloud is empty: " + path.string());
    }
    return TorchUtility::ToTensor1D(points, 3);
}

void DiffusionAI::SavePointCloud(const std::filesystem::path& path,
                                 const torch::Tensor& points) const
{
    namespace fs = std::filesystem;
    if (!path.parent_path().empty()) {
        fs::create_directories(path.parent_path());
    }

    auto cpuPoints = points.detach().cpu().contiguous().to(torch::kFloat32);
    if (cpuPoints.dim() == 3) {
        cpuPoints = cpuPoints.squeeze(0);
    }
    if (cpuPoints.dim() != 2 || cpuPoints.size(1) != 3) {
        throw std::runtime_error("Generated tensor must have shape [N, 3].");
    }

    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open output file: " + path.string());
    }
    const int pointCount = static_cast<int>(cpuPoints.size(0));
    const int hasNormal = 0;
    const int hasColor = 0;
    ofs.write(reinterpret_cast<const char*>(&pointCount), sizeof(pointCount));
    ofs.write(reinterpret_cast<const char*>(&hasNormal), sizeof(hasNormal));
    ofs.write(reinterpret_cast<const char*>(&hasColor), sizeof(hasColor));
    ofs.write(reinterpret_cast<const char*>(cpuPoints.data_ptr<float>()),
              sizeof(float) * cpuPoints.numel());
    if (!ofs) {
        throw std::runtime_error("Failed to write output file: " + path.string());
    }
}

torch::Device DiffusionAI::GetDevice() const
{
    if (torch::cuda::is_available()) {
        return torch::Device(torch::kCUDA);
    }
    return torch::Device(torch::kCPU);
}

void DiffusionAI::Train(const std::string& path)
{
    namespace fs = std::filesystem;
    const fs::path inputPath(path);
    if (!fs::exists(inputPath) || !fs::is_regular_file(inputPath)) {
        throw std::runtime_error(
            "Training input must be one point cloud file: " + path);
    }
    auto clean = LoadPointCloud(inputPath).unsqueeze(0);
    const int64_t pointCount = clean.size(1);

    fs::path modelPath(inputPath);
    modelPath.replace_extension(".diffusion.pt");
    const auto device = GetDevice();

    if (fs::exists(modelPath)) {
        std::cout << "Load By File: " << modelPath << std::endl;
        m_model = DiffusionModel();
        torch::load(m_model, modelPath.string());

		const int64_t modelPointCount = m_model->GetPointCount();
		if (modelPointCount <= 0) {
			throw std::runtime_error(
				"The model does not contain a valid point count.");
		}
		m_model->to(device);
		m_model->eval();
		m_predictPoint = torch::randn({ 1, modelPointCount, 3 },
			torch::TensorOptions().dtype(torch::kFloat32).device(device));
		m_predictStep = DiffusionSteps - 1;
		m_modelReady = true;
        return;
    }

    m_model = DiffusionModel();
    m_model->SetPointCount(pointCount);
    m_model->to(device);
    m_model->train();
    clean = clean.to(device);

    auto betas = torch::linspace(1.0e-4, 2.0e-2, DiffusionSteps,
        torch::TensorOptions().dtype(torch::kFloat32).device(device));
    auto alphaBars = torch::cumprod(1.0f - betas, 0);
    torch::optim::Adam optimizer(m_model->parameters(),
        torch::optim::AdamOptions(1.0e-3));

    for (int64_t epoch = 0; epoch < EpochCount; ++epoch) {
        auto timestep = torch::randint(0, DiffusionSteps, { 1 },
            torch::TensorOptions().dtype(torch::kInt64).device(device));
        auto noise = torch::randn_like(clean);
        auto selectedAlphaBar = alphaBars.index_select(0, timestep)
            .reshape({ 1, 1, 1 });
        auto noisy = selectedAlphaBar.sqrt() * clean +
            (1.0f - selectedAlphaBar).sqrt() * noise;
        auto normalizedTime = timestep.to(torch::kFloat32) /
            static_cast<float>(DiffusionSteps - 1);
        auto predictedNoise = m_model->forward(noisy, normalizedTime);
        auto loss = torch::mse_loss(predictedNoise, noise);

        optimizer.zero_grad();
        loss.backward();
        optimizer.step();

        if (epoch % 10 == 0 || epoch + 1 == EpochCount) {
            std::cout << "Epoch: " << epoch
                << ", Loss: " << loss.item<float>()
                << std::endl;
        }
    }

    m_model->to(torch::kCPU);
    torch::save(m_model, modelPath.string());
    std::cout << "Save File: " << modelPath << std::endl;

	m_model->to(device);
	m_model->eval();
	m_predictPoint = torch::randn({ 1, pointCount, 3 },
		torch::TensorOptions().dtype(torch::kFloat32).device(device));
	m_predictStep = DiffusionSteps - 1;
	m_modelReady = true;
}

void DiffusionAI::Predict(const std::string& /*modelPath*/, const std::string& outPath)
{
	if (!m_modelReady || !m_model) {
		throw std::runtime_error(
			"Diffusion model is not initialized. Run --train first.");
	}

    const auto device = GetDevice();
    const int64_t pointCount = m_model->GetPointCount();
    if (pointCount <= 0) {
        throw std::runtime_error("The model does not contain a valid point count.");
    }


    torch::NoGradGuard noGrad;
    auto betas = torch::linspace(1.0e-4, 2.0e-2, DiffusionSteps,
        torch::TensorOptions().dtype(torch::kFloat32).device(device));
    auto alphas = 1.0f - betas;
    auto alphaBars = torch::cumprod(alphas, 0);

    const int64_t step = m_predictStep;
    auto timestep = torch::full({ 1 },
        static_cast<float>(step) / static_cast<float>(DiffusionSteps - 1),
        torch::TensorOptions().dtype(torch::kFloat32).device(device));
    auto predictedNoise = m_model->forward(m_predictPoint, timestep);
    auto beta = betas[step];
    auto alpha = alphas[step];
    auto alphaBar = alphaBars[step];
    auto mean = (m_predictPoint - beta / (1.0f - alphaBar).sqrt() *
        predictedNoise) / alpha.sqrt();
    m_predictPoint = step > 0
        ? mean + beta.sqrt() * torch::randn_like(m_predictPoint)
        : mean;
    --m_predictStep;

    SavePointCloud(outPath, m_predictPoint);
    std::cout << "Save Point Cloud: " << outPath
		<< ", diffusion step: " << step << std::endl;
}

}

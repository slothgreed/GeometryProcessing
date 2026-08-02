#include "SDFAI.h"
#include "TorchUtility.h"
#include <torch/torch.h>
#include <fstream>
#include <iostream>
#include <filesystem>
namespace KI
{
std::vector<float> SDFAI::VoxelData::CreatePosition() const
{
    std::vector<float> position;
    position.reserve(
        static_cast<size_t>(resolution[0]) *
        static_cast<size_t>(resolution[1]) *
        static_cast<size_t>(resolution[2]) * 3);

    const float pitchX = (max[0] - min[0]) / static_cast<float>(resolution[0]);
    const float pitchY = (max[1] - min[1]) / static_cast<float>(resolution[1]);
    const float pitchZ = (max[2] - min[2]) / static_cast<float>(resolution[2]);

    const float halfX = pitchX * 0.5f;
    const float halfY = pitchY * 0.5f;
    const float halfZ = pitchZ * 0.5f;

    for (int x = 0; x < resolution[0]; ++x)
    for (int y = 0; y < resolution[1]; ++y)
    for (int z = 0; z < resolution[2]; ++z) {
		position.push_back(min[0] + x * pitchX + halfX);
		position.push_back(min[1] + y * pitchY + halfY);
		position.push_back(min[2] + z * pitchZ + halfZ);
    }

    return position;
}
SDFAI::VoxelData SDFAI::Load(const std::string& path)
{
    VoxelData voxel;
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) return SDFAI::VoxelData();

    // Min
    ifs.read(reinterpret_cast<char*>(voxel.min), sizeof(float) * 3);

    // Max
    ifs.read(reinterpret_cast<char*>(voxel.max), sizeof(float) * 3);

    // Resolution
    ifs.read(reinterpret_cast<char*>(voxel.resolution), sizeof(int) * 3);

    int count =
        voxel.resolution[0] *
        voxel.resolution[1] *
        voxel.resolution[2];

    voxel.data.resize(count);

    ifs.read(
        reinterpret_cast<char*>(voxel.data.data()),
        sizeof(float) * count);

    return voxel;
}

void SDFAI::Train(const std::string& path)
{
    namespace fs = std::filesystem;

    // 学習済みモデルのパス
    fs::path modelPath(path);
    modelPath.replace_extension(".pt");

    //---------------------------------------------------------------------
    // 学習済みモデルが存在するなら読み込んで終了
    //---------------------------------------------------------------------
    if (fs::exists(modelPath)) {
        std::cout << "Load By File : " << modelPath << std::endl;
        torch::load(m_model, modelPath.string());
        return;
    }

	auto voxel = Load(path);
    auto posTensor = TorchUtility::ToTensor1D(voxel.CreatePosition(), 3);
	auto sdfTensor = TorchUtility::ToTensor1D(voxel.data, 1);
    torch::optim::Adam optimizer(m_model->parameters(), 0.001);
    for (int epoch = 0; epoch < 1000; ++epoch) {
        auto outputs = m_model->forward(posTensor);          // 推論
        auto loss = torch::mse_loss(outputs, sdfTensor);    // 誤差計算
		optimizer.zero_grad();                              // 勾配の初期化
		loss.backward();                                    // 誤差逆伝播(勾配更新)
		optimizer.step();                                   // パラメータをOptimizerの数式で更新
        if (epoch % 10 == 0) {
            std::cout
                << "Epoch: " << epoch
                << ", Loss: " << loss.item<float>()
                << ", Output[min, max, mean]: "
                << outputs.min().item<float>() << ", "
                << outputs.max().item<float>() << ", "
                << outputs.mean().item<float>()
                << ", Target[min, max, mean]: "
                << sdfTensor.min().item<float>() << ", "
                << sdfTensor.max().item<float>() << ", "
                << sdfTensor.mean().item<float>()
                << '\n';
        }
    }
    //---------------------------------------------------------------------
    // 保存
    //---------------------------------------------------------------------
    std::cout << "Save File : " << modelPath << std::endl;
    torch::save(m_model, modelPath.string());
}

static std::vector<std::string> Parse(const std::string& command)
{
    std::vector<std::string> tokens;

    std::stringstream ss(command);
    std::string token;

    while (ss >> token) {
        tokens.push_back(std::move(token));
    }

    for(size_t i = 0; i < tokens.size(); ++i) {
        std::cout << "Token[" << i << "]: " << tokens[i] << std::endl;
	}
    return tokens;
}

bool SDFAI::Execute(const std::string& command)
{
	auto parsed = Parse(command);
	if (parsed.size() == 0) return false;

    if (parsed[0] != GetCommandName()) { return false; }
	const std::string& action = parsed[1];
	if (action == "--train") {
		Train(parsed[2]);
        return true;
    } else if (action == "--predict") {
        Predict(parsed[2], parsed[3]);
        return true;
    }

    return false;
}

void SDFAI::Predict(const std::string& in, const std::string& out)
{
    std::ifstream ifs(in, std::ios::binary);
    if (!ifs) {
		std::cout << "Failed to open input file: " << in << std::endl;
        return;
    }
    ifs.seekg(0, std::ios::end);
    const auto size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

	if (size <= 0 || size % sizeof(float) != 0) return;

	std::vector<float> position(size / sizeof(float));
	ifs.read(reinterpret_cast<char*>(position.data()), size);

	if (position.size() % 3 != 0) return;

	auto result = Predict(position);

	// write binary float array
	std::ofstream ofs(out, std::ios::binary);
    if (!ofs) {
        std::cout << "Failed to open output file: " << out << std::endl;
        return;
    }
	ofs.write(reinterpret_cast<const char*>(result.data()),
		result.size() * sizeof(float));
}
std::vector<float> SDFAI::Predict(const std::vector<float>& position)
{
    torch::NoGradGuard noGrad;

    auto input = TorchUtility::ToTensor1D(position, 3);
    auto output = m_model->forward(input);
    return TorchUtility::ToFloatVector(output);
}

}
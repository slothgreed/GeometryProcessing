#include <unordered_set>
#include <torch/torch.h>
#include <iostream>
#include <Windows.h>
#include <string_view>
#include "ServerPipe.h"
void standalone()
{
    auto x = torch::rand({ 4, 1 });
    auto y = 2.0f * x + 1.0f;

    auto linear = torch::nn::Linear(1, 1);
    torch::optim::SGD optimizer(linear->parameters(), 0.1);

    for (int i = 0; i < 100; ++i) {
        auto pred = linear->forward(x);
        auto loss = torch::mse_loss(pred, y);

        optimizer.zero_grad();
        loss.backward();
        optimizer.step();

        if (i % 10 == 0)
            std::cout << i << " loss = " << loss.item<float>() << std::endl;
    }


    auto weight_tensor = linear->weight.detach();
    auto bias_tensor = linear->bias.detach();
    weight_tensor = weight_tensor.cpu();
    bias_tensor = bias_tensor.cpu();
    float weight_value = weight_tensor.squeeze().item<float>();
    float bias_value = bias_tensor.squeeze().item<float>();

    std::cout << "weight: " << weight_value << std::endl;
    std::cout << "bias: " << bias_value << std::endl;
}


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

torch::Tensor ToTensor(const std::vector<float>& data, int dimension)
{
    // 修正: 行数は data.size() / dimension でなければならない
    int64_t rows = static_cast<int64_t>(data.size()) / dimension;
    int64_t cols = static_cast<int64_t>(dimension);
    return torch::from_blob(
        const_cast<float*>(data.data()),
        { rows, cols },
        torch::TensorOptions().dtype(torch::kFloat32)
    ).clone();
}

std::vector<float> ToFloatVector(torch::Tensor tensor)
{
    // GPU Tensor の可能性があるので CPU へ移す
    tensor = tensor.detach().cpu().contiguous().to(torch::kFloat32);

    // 要素数を取得
    size_t size = tensor.numel();

    std::vector<float> result(size);
    // Tensorの中身をvectorへコピー
	std::memcpy(result.data(), tensor.data_ptr<float>(), size * sizeof(float));

    return result;
}

void NeuralSDFTest(const std::vector<float>& position, const std::vector<float>& sdf)
{
	auto positionTensor = ToTensor(position, 3);
	auto sdfTensor = ToTensor(sdf, 1);
    NeuralSDFImpl model;
    torch::optim::Adam optimizer(model.parameters(), 0.001);
    for (int epoch = 0; epoch < 100; ++epoch) {
        auto outputs = model.forward(positionTensor);
        auto loss = torch::mse_loss(outputs, sdfTensor);
        optimizer.zero_grad();
        loss.backward();
        optimizer.step();
        if (epoch % 10 == 0)
            std::cout << "Epoch [" << epoch << "/100], Loss: " << loss.item<float>() << std::endl;
    }
}

struct Options
{
    bool named = false;
};

Options parseArgs(int argc, char* argv[])
{
    Options opt;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);

        if (arg == "--named") {
            opt.named = true;
        }
    }

    return opt;
}



int main(int argc, char* argv[])
{
	auto args = parseArgs(argc, argv);
    if (args.named) {
		KI::ServerPipe server;
        server.Connect();
    } else {
        standalone();
    }
    return 0;
}
#include <unordered_set>
#include <torch/torch.h>
#include <iostream>
#include <Windows.h>
#include <string_view>
#include "ServerPipe.h"
#include "SDFAI.h"
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


struct Options
{
    bool named = false;
};

Options parseArgs(int argc, char* argv[])
{
    Options opt;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
		std::cout << "arg: " << arg << std::endl;
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
        server.SetReceiveCommand(std::make_shared<KI::SDFAI>());
        server.Connect();
    } else {
        standalone();
    }
    return 0;
}
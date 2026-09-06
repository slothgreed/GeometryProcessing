#ifndef MODEL_CLASSIFICATION_AI_H
#define MODEL_CLASSIFICATION_AI_H
#include "ICommandReceiver.h"
#include <string>
#include <torch/torch.h>
#include <tuple>
namespace KI
{

class ModelClassificationAI : public ICommandReceiver
{
private:

    struct ModelClassificationImpl : torch::nn::Module
    {
        ModelClassificationImpl(int classCount = 40)
        {
            pointMLP = register_module(
                "pointMLP",
                torch::nn::Sequential(
                    torch::nn::Linear(3, 64),
                    torch::nn::ReLU(),
                    torch::nn::Linear(64, 128),
                    torch::nn::ReLU(),
                    torch::nn::Linear(128, 256),
                    torch::nn::ReLU()
                ));

            classifier = register_module(
                "classifier",
                torch::nn::Sequential(
                    torch::nn::Linear(256, 128),
                    torch::nn::ReLU(),
                    torch::nn::Linear(128, classCount)
                ));
        }

        torch::Tensor forward(torch::Tensor points)
        {
            /*
             * points:
             * [batchSize, pointCount, 3]
             */

            auto features = pointMLP->forward(points);

            /*
             * features:
             * [batchSize, pointCount, 256]
             */

            auto result = (torch::max)(features, 1);
            auto pooled = std::get<0>(result);

            /*
             * pooled:
             * [batchSize, 256]
             *
             * pointCount方向の最大値を取る。
             */

            auto logits = classifier->forward(pooled);

            /*
             * logits:
             * [batchSize, classCount]
             */

            return logits;
        }

        torch::nn::Sequential pointMLP{ nullptr };
        torch::nn::Sequential classifier{ nullptr };
    };

    TORCH_MODULE(ModelClassification);
public:
    ModelClassificationAI();
	~ModelClassificationAI();
    virtual std::string GetCommandName() const { return std::string("ModelClassificationAI"); }
	virtual bool Execute(const std::string& command) override;
    void Test();

private:
    void Predict(const std::string& in, const std::string& out);
    int Predict(const std::vector<float>& position);
    void Train(const std::string& path);
    ModelClassification m_model;
    torch::Device m_device;

};
}


#endif MODEL_CLASSIFICATION_AI_H

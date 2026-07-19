#ifndef TORCH_UTILITY_H
#define TORCH_UTILITY_H
#include <torch/torch.h>
namespace KI
{
class TorchUtility
{
public:
	TorchUtility() {};
	~TorchUtility() {};

	static torch::Tensor ToTensor3D(const std::vector<float>& data, int resolution);
	static torch::Tensor ToTensor1D(const std::vector<float>& data, int dimension);
	static std::vector<float> ToFloatVector(torch::Tensor tensor);

private:

};

}

#endif TORCH_UTILITY_H
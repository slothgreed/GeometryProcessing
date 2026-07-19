#include "TorchUtility.h"
namespace KI
{
torch::Tensor TorchUtility::ToTensor3D(const std::vector<float>& data, int resolution)
{
    return torch::from_blob(
        const_cast<float*>(data.data()),
        {
            static_cast<int64_t>(resolution),
            static_cast<int64_t>(resolution),
            static_cast<int64_t>(resolution)
        },
        torch::TensorOptions().dtype(torch::kFloat32)
    ).clone();
}

torch::Tensor TorchUtility::ToTensor1D(const std::vector<float>& data, int dimension)
{
    assert(dimension > 0);
    assert(data.size() % dimension == 0);

    const int64_t rows = static_cast<int64_t>(data.size() / dimension);
    const int64_t cols = static_cast<int64_t>(dimension);

    return torch::from_blob(
        const_cast<float*>(data.data()),
        { rows, cols },
        torch::TensorOptions().dtype(torch::kFloat32)
    ).clone();
}

std::vector<float> TorchUtility::ToFloatVector(torch::Tensor tensor)
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
}
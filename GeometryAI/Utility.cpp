#include "Utility.h"
#include <cmath>    
#include <fstream>
#include <filesystem>
namespace KI
{
GeometryUtility::GeometryUtility()
{
}
GeometryUtility::~GeometryUtility()
{
}

std::vector<float> GeometryUtility::NormalizePointCloud(const std::vector<float>& points)
{
    if (points.empty() || points.size() % 3 != 0) {
        return {};
    }

    const size_t pointCount = points.size() / 3;

    // 重心計算
    float cx = 0.0f;
    float cy = 0.0f;
    float cz = 0.0f;

    for (size_t i = 0; i < pointCount; ++i) {
        cx += points[i * 3 + 0];
        cy += points[i * 3 + 1];
        cz += points[i * 3 + 2];
    }

    cx /= static_cast<float>(pointCount);
    cy /= static_cast<float>(pointCount);
    cz /= static_cast<float>(pointCount);

    // 原点からの最大距離を求める
    float maxDistance = 0.0f;

    for (size_t i = 0; i < pointCount; ++i) {
        const float x = points[i * 3 + 0] - cx;
        const float y = points[i * 3 + 1] - cy;
        const float z = points[i * 3 + 2] - cz;

        const float dist = std::sqrt(x * x + y * y + z * z);
        maxDistance = std::max(maxDistance, dist);
    }

    if (maxDistance == 0.0f) {
        return points;
    }

    // 正規化
    std::vector<float> normalized(points.size());

    const float invScale = 1.0f / maxDistance;

    for (size_t i = 0; i < pointCount; ++i) {
        normalized[i * 3 + 0] = (points[i * 3 + 0] - cx) * invScale;
        normalized[i * 3 + 1] = (points[i * 3 + 1] - cy) * invScale;
        normalized[i * 3 + 2] = (points[i * 3 + 2] - cz) * invScale;
    }

    return normalized;
}

std::vector<float> GeometryUtility::LoadPointCloud(const std::string& filePath)
{
    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("File not found : " + filePath);
    }

    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file : " + filePath);
    }

    int pointNum = 0;
    int normalComponentNum = 0;
    int colorComponentNum = 0;

    ifs.read(reinterpret_cast<char*>(&pointNum), sizeof(pointNum));
    ifs.read(reinterpret_cast<char*>(&normalComponentNum), sizeof(normalComponentNum));
    ifs.read(reinterpret_cast<char*>(&colorComponentNum), sizeof(colorComponentNum));

    if (!ifs) {
        throw std::runtime_error("Failed to read header : " + filePath);
    }

    std::vector<float> points(pointNum * 3);

    ifs.read(reinterpret_cast<char*>(points.data()), sizeof(float) * points.size());

    if (!ifs) {
        throw std::runtime_error("Failed to read point data : " + filePath);
    }

    return points;
}
}

#include "GeometryUtility.h"
#include "Utility.h"
namespace KI
{
unsigned int MortonCode::To(unsigned int x)
{
    // Mortonコードの計算（各ビットを交互に並べる）
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;
    return x;
}

Vector3 MortonCode::ToColor(unsigned int morton)
{
    return ColorUtility::CreatePrimary(morton & 0b111);
}
void MortonCode::Create(const Vector<Vector3>& position, const Vector<unsigned int>& triangle, const BDB& box, int resolute)
{
    resolute = resolute - 1;
    auto triangleNum = triangle.size() / 3;
    m_mortons.resize(triangleNum);
    for (int i = 0; i < triangleNum; i++) {
        const auto& p0 = position[triangle[3 * i]];
        const auto& p1 = position[triangle[3 * i + 1]];
        const auto& p2 = position[triangle[3 * i + 2]];
        auto centroid = (p0 + p1 + p2) / 3.0f;
        auto morton = (centroid - box.Min()) / (box.Max() - box.Min()) * (float)resolute;
        auto x = unsigned int(morton.x);
        auto y = unsigned int(morton.y);
        auto z = unsigned int(morton.z);

        m_mortons[i].box.Add(p0); m_mortons[i].box.Add(p1); m_mortons[i].box.Add(p2);
        m_mortons[i].triangleIndex = i;
        m_mortons[i].morton = To(x) | (To(y) << 1) | (To(z) << 2);
    }

    std::sort(m_mortons.begin(), m_mortons.end(), [&](const auto& a, const auto& b)
    {
        return a.morton < b.morton;
    });
}

}
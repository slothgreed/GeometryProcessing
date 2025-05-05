#include "GeometryUtility.h"
#include "Utility.h"
#include "HalfEdgeNode.h"
#include "HalfEdgeStruct.h"
#include "Utility.h"
#include "Voxelizer.h"
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

unsigned int MortonCode::To(const Vector3& x)
{
    return To((unsigned int)x.x) | (To((unsigned int)x.y) << 1) | (To((unsigned int)x.z) << 2);
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
        m_mortons[i].morton = To(morton);
    }

    std::sort(m_mortons.begin(), m_mortons.end(), [&](const auto& a, const auto& b)
    {
        return a.morton < b.morton;
    });
}

Vector<Vector3> MeshAlgorithm::CreatePoissonSampleVolume(HalfEdgeNode& halfEdge)
{
    Voxelizer voxelizer(&halfEdge);

    const auto& bdb = halfEdge.GetBoundBox();
    voxelizer.Execute(1 << 6); // TODO : 適切な値 
    Vector<Vector3> poisson;
    for (int i = 0; i < voxelizer.GetResolution(); i++)
    for (int j = 0; j < voxelizer.GetResolution(); j++)
    for (int k = 0; k < voxelizer.GetResolution(); k++) {
        auto label = voxelizer.GetLabel(Vector3(i, j, k));
        if (label == Voxelizer::INNER) {
            auto cell = voxelizer.GetCellBDB(Vector3(i, j, k));
            auto x = Random::Float(cell.Min().x, cell.Max().x);
            auto y = Random::Float(cell.Min().y, cell.Max().y);
            auto z = Random::Float(cell.Min().z, cell.Max().z);
            poisson.push_back(Vector3(x, y, z));
        }
    }

    return poisson;
}
Vector<Vector3> MeshAlgorithm::CreatePoissonSampleOnFace(const HalfEdgeStruct& halfEdge)
{
    std::vector<float> areas(halfEdge.GetFaceNum());
    std::vector<float> cdf(halfEdge.GetFaceNum()); // 累積分布関数
    float sumArea = 0.0f;
    for (int i = 0; i < halfEdge.GetFaceNum(); i++) {
        areas[i] = halfEdge.CalcFaceArea(i);
        sumArea += areas[i];
        cdf[i] = sumArea;
    }

    Vector<Vector3> samples;
    int poissonNum = 10000;
    float r = 0.1f;
    for (int i = 0; i < poissonNum; i++) {
        float randomArea = Random::Float(0.0f, 1.0f) * sumArea;
        auto itr = std::lower_bound(cdf.begin(), cdf.end(), randomArea);
        int index = std::distance(cdf.begin(), itr);
        auto face = halfEdge.GetFace(index);

        Vector3 randomPoint;
        {
            float u = Random::Float(0.0f, 1.0f);
            float v = Random::Float(0.0f, 1.0f);

            if (u + v > 1.0f) { u = 1.0f - u; v = 1.0f - v; }
            randomPoint = face.pos0 + u * (face.pos1 - face.pos0) + v * (face.pos2 - face.pos0);
        }
        

        bool valid = true;
        for (const auto& s : samples) {
            if ((randomPoint- s).length() < r) {
                valid = false;
                i = -1;
                break;
            }
        }

        if (valid) {
            samples.push_back(randomPoint);
        }
    }

    return samples;
}
}
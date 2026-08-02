#include "GeometryLoader.h"
#include "FileUtility.h"
#include "Mesh.h"
namespace KI
{

Mesh* GeometryLoader::LoadOff(const String& name)
{
	Vector<Vector3> position;
	Vector<UInt> indices;
	int vertexNum = 0;
	int faceNum = 0;
	int edgeNum = 0;
    int loadedVertexNum = 0;
    int loadedFaceNum = 0;
	MemoryMappedFile mapped;
	mapped.Open(name);
	auto text = mapped.Text();
	for (auto line : LineReader(text)) {
		auto str = std::string(line);
		if (str.empty()) continue;
		if (str[0] == '#') continue;
		if (str.rfind("OFF", 0) == 0) {
			str.erase(0, 3);
			if (str.empty()) continue;
		}
		auto text = StringUtility::Split(str, ' ');
		if (vertexNum == 0) {
			if (text.size() < 3) throw std::runtime_error("Invalid OFF count line.");
			vertexNum = StringUtility::ToInt(text[0]);
			faceNum = StringUtility::ToInt(text[1]);
			edgeNum = StringUtility::ToInt(text[2]);
			position.reserve(vertexNum);
			indices.reserve(faceNum * 3);
			continue;
		}

		if (loadedVertexNum < vertexNum) {
			if (text.size() < 3)
				throw std::runtime_error("Invalid OFF vertex line.");

			const float x = StringUtility::ToFloat(text[0]);
			const float y = StringUtility::ToFloat(text[1]);
			const float z = StringUtility::ToFloat(text[2]);

			position.push_back(Vector3(x, y, z));
            ++loadedVertexNum;

			continue;
		}

        if (loadedFaceNum < faceNum) {
			const int polygonVertexNum = StringUtility::ToInt(text[0]);
			std::vector<uint32_t> faceIndices;
			faceIndices.reserve(polygonVertexNum);

			for (int i = 0; i < polygonVertexNum; ++i) {
				faceIndices.push_back(StringUtility::ToInt(text[i + 1]));
			}

			// GL_TRIANGLES用のインデックスへ変換
			for (int i = 1; i < polygonVertexNum - 1; ++i) {
				indices.push_back(faceIndices[0]);
				indices.push_back(faceIndices[i]);
				indices.push_back(faceIndices[i + 1]);
			}

            ++loadedFaceNum;
            continue;
        }
	}
	mapped.Close();

	auto pMesh = new Mesh(std::move(position),std::move(indices),Mesh::DrawType::Triangles);
	pMesh->BuildNormal();
	return pMesh;
}
}
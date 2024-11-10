#include "HalfEdgeStruct.h"
#include "BDB.h"
namespace KI
{


HalfEdgeStruct::Edge HalfEdgeStruct::GetEdge(int edgeIndex) const
{
	HalfEdgeStruct::Edge edge;
	edge.begin = m_position[m_halfEdge[m_halfEdge[edgeIndex].oppositeEdge].endPos];
	edge.end = m_position[m_halfEdge[edgeIndex].endPos];
	return edge;
}


HalfEdgeStruct::IndexedEdge HalfEdgeStruct::GetIndexedEdge(int edgeIndex) const
{
	HalfEdgeStruct::IndexedEdge edge;
	edge[0] = m_halfEdge[m_halfEdge[edgeIndex].oppositeEdge].endPos;
	edge[1] = m_halfEdge[edgeIndex].endPos;
	return edge;
}

const HalfEdgeStruct::IndexedFace& HalfEdgeStruct::GetIndexedFace(int faceIndex) const
{
	return m_face[faceIndex];
}

HalfEdgeStruct::Face HalfEdgeStruct::GetFace(int faceIndex) const
{
	auto index = GetIndexedFace(faceIndex);
	Face face;
	face.pos0 = m_position[index.position[0]];
	face.pos1 = m_position[index.position[1]];
	face.pos2 = m_position[index.position[2]];
	return face;
}

Vector3 HalfEdgeStruct::CalcGravity(const IndexedFace& face) const
{
	auto position = m_position[face.position[0]];
	position += m_position[face.position[1]];
	position += m_position[face.position[2]];
	position *= (1/3);
	return position;
}

Vector3 HalfEdgeStruct::CalcFaceNormal(int faceIndex) const
{
	auto p0 = m_position[m_face[faceIndex].position[0]];
	auto p1 = m_position[m_face[faceIndex].position[1]];
	auto p2 = m_position[m_face[faceIndex].position[2]];

	auto v1 = p1 - p0;
	auto v2 = p2 - p0;

	return glm::normalize(glm::cross(v1, v2));
}

Vector<unsigned int> HalfEdgeStruct::GetAroundFace(const IndexedFace& face) const
{
	Vector<unsigned int> aroundFace(3);

	aroundFace[0] = m_halfEdge[m_halfEdge[face.edge[0]].oppositeEdge].face;
	aroundFace[1] = m_halfEdge[m_halfEdge[face.edge[1]].oppositeEdge].face;
	aroundFace[2] = m_halfEdge[m_halfEdge[face.edge[2]].oppositeEdge].face;

	return aroundFace;
}
Vector<int> HalfEdgeStruct::GetAroundEdge(int posIndex) const
{
	Vector<int> loopEdge;

	int edgeIndex = m_positionToEdge[posIndex];
	loopEdge.push_back(edgeIndex);
	while (true) {
		const auto& edge = m_halfEdge[edgeIndex];
		edgeIndex = m_halfEdge[edge.oppositeEdge].nextEdge;
		if (edgeIndex == loopEdge[0]) {
			break;
		}
		loopEdge.push_back(edgeIndex);
	}

	return loopEdge;
}

Vector<int> HalfEdgeStruct::GetAroundFaceFromPosition(int posIndex) const
{
	auto aroundEdge = GetAroundEdge(posIndex);
	Vector<int> aroundFace;
	for (auto edge : aroundEdge) {
		aroundFace.push_back(m_halfEdge[edge].face);
	}

	return aroundFace;
}
Vector<unsigned int> HalfEdgeStruct::CreateIndexBufferData() const
{
	Vector<unsigned int> indexBuffer;
	indexBuffer.resize(m_faceToEdge.size() * 3);
	int counter = 0;
	for (const auto& face : m_face) {
		indexBuffer[counter++] = face.position[0];
		indexBuffer[counter++] = face.position[1];
		indexBuffer[counter++] = face.position[2];
	}

	return indexBuffer;
}

Vector<unsigned int> HalfEdgeStruct::CreateEdgeIndexBufferData()
{
	Vector<unsigned int> indexBuffer;
	indexBuffer.resize(m_halfEdge.size() * 2);
	for (size_t i = 0; i < m_halfEdge.size(); i++) {
		auto edge = GetIndexedEdge(i);
		indexBuffer[2 * i] = edge[0];
		indexBuffer[2 * i + 1] = edge[1];
	}

	return indexBuffer;
}

BDB HalfEdgeStruct::CreateBDB() const
{
	BDB bdb;
	for (const auto& pos : m_position) {
		bdb.Add(pos);
	}
	return bdb;
}

void HalfEdgeStruct::CreateFace()
{
	m_face.resize(m_faceToEdge.size());
	int counter = 0;
	for (auto edgeIndex : m_faceToEdge) {
		auto face = IndexedFace();
		const auto& edge = m_halfEdge[edgeIndex];
		face.edge[0] = edgeIndex;
		face.edge[1] = edge.beforeEdge;
		face.edge[2] = edge.nextEdge;

		face.position[0] = m_halfEdge[face.edge[0]].endPos;
		face.position[1] = m_halfEdge[face.edge[1]].endPos;
		face.position[2] = m_halfEdge[face.edge[2]].endPos;
		m_face[counter++] = std::move(face);
	}
}
const Vector<Vector3>& HalfEdgeStruct::GetNormal()
{
	if (m_normal.size() == 0) { CreateNormal(); }
	return m_normal;
}
Vector3 HalfEdgeStruct::GetNormal(int index)
{
	if (m_normal.size() == 0) {
		CreateNormal();
	}

	return m_normal[index];
}
void HalfEdgeStruct::CreateNormal()
{
	m_normal.resize(m_position.size());
	for (size_t i = 0; i < m_position.size(); i++) {
		auto faces = GetAroundFaceFromPosition(i);
		Vector3 normal(0);
		for (size_t j = 0; j < faces.size(); j++) {
			normal += CalcFaceNormal(faces[j]);
		}
		normal.x = normal.x / faces.size();
		normal.y = normal.y / faces.size();
		normal.z = normal.z / faces.size();
		m_normal[i] = glm::normalize(normal);
	}
}

Vector<Vector3> HalfEdgeStruct::ConvertVertexColorToFaceColor(const Vector<Vector3>& vertexColor) const
{
	Vector<Vector3> faceColor(GetFaceNum() * 3);
	for (size_t i = 0; i < GetFaceNum(); i++) {
		const auto& face = GetIndexedFace(i);
		faceColor[3 * i] = (vertexColor[face.position[0]]);
		faceColor[3 * i + 1] = (vertexColor[face.position[1]]);
		faceColor[3 * i + 2] = (vertexColor[face.position[2]]);
	}

	return faceColor;
}

}
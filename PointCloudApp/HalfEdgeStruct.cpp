#include "HalfEdgeStruct.h"
#include "BDB.h"
namespace KI
{


const Vector3& HalfEdgeStruct::GetBegin(int edgeIndex) const
{
	return m_position[m_halfEdge[edgeIndex].endPos];
}

const Vector3& HalfEdgeStruct::GetEnd(int edgeIndex) const
{
	return
		m_position[
			m_halfEdge[m_halfEdge[edgeIndex].oppositeEdge].endPos];
}

IndexedEdge HalfEdgeStruct::GetEdge(int edgeIndex) const
{
	IndexedEdge edge;
	edge[0] = m_halfEdge[m_halfEdge[edgeIndex].oppositeEdge].endPos;
	edge[1] = m_halfEdge[edgeIndex].endPos;

	return edge;
}

const HalfEdgeStruct::Face& HalfEdgeStruct::GetFace(int faceIndex) const
{
	return m_face[faceIndex];
}

Vector3 HalfEdgeStruct::CalcGravity(const Face& face) const
{
	auto position = m_position[face.position[0]];
	position += m_position[face.position[1]];
	position += m_position[face.position[2]];
	position *= (1/3);
	return position;
}

Vector<unsigned int> HalfEdgeStruct::GetAroundFace(const Face& face) const
{
	Vector<unsigned int> aroundFace(3);

	aroundFace[0] = m_halfEdge[m_halfEdge[face.edge[0]].oppositeEdge].face;
	aroundFace[1] = m_halfEdge[m_halfEdge[face.edge[1]].oppositeEdge].face;
	aroundFace[2] = m_halfEdge[m_halfEdge[face.edge[2]].oppositeEdge].face;

	return aroundFace;
}
Vector<int> HalfEdgeStruct::GetOneLoopEdge(int posIndex) const
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
		auto edge = GetEdge(i);
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
		auto face = Face();
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
}
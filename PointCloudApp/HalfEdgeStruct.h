#ifndef HALF_EDGE_STRUCT_H
#define HALF_EDGE_STRUCT_H
#include "BDB.h"
namespace KI
{
struct HalfEdge
{
	int endPos;
	int nextEdge;
	int beforeEdge;
	int oppositeEdge;
	int face;
};

typedef std::array<int, 2> IndexedEdge;


class HalfEdgeStruct
{
public:

	struct Face
	{
		std::array<int, 3> position;
		std::array<int, 3> edge;
	};
	HalfEdgeStruct() {};
	~HalfEdgeStruct() {};

	void Set(
		Vector<Vector3>&& position,
		Vector<HalfEdge>&& halfEdge,
		Vector<int>&& positionToEdge,
		Vector<int>&& faceToEdge)
	{
		m_position = std::move(position);
		m_halfEdge = std::move(halfEdge);
		m_positionToEdge = std::move(positionToEdge);
		m_faceToEdge = std::move(faceToEdge);
		CreateFace();
	}

	size_t GetFaceNum() const { return m_faceToEdge.size(); }
	size_t GetPositionNum() const { return m_position.size(); }
	size_t GetEdgeNum() const { return m_halfEdge.size(); }
	const Vector<Vector3>& GetPosition() { return m_position; }
	const Vector3& GetBegin(int edgeIndex) const;
	const Vector3& GetEnd(int edgeIndex) const;
	IndexedEdge GetEdge(int edgeIndex) const;
	const Face& GetFace(int faceIndex) const;
	Vector<int> GetOneLoopEdge(int positionIndex) const;
	Vector<unsigned int> CreateIndexBufferData() const;
	Vector<unsigned int> CreateEdgeIndexBufferData();
	BDB CreateBDB() const;
	Vector<unsigned int> GetAroundFace(const Face& triangle) const;
	Vector3 CalcGravity(const Face& triangle) const;

private:
	void CreateFace();
	Vector<HalfEdgeStruct::Face> m_face;
	Vector<HalfEdge> m_halfEdge;
	Vector<Vector3> m_position;
	Vector<int> m_positionToEdge;
	Vector<int> m_faceToEdge;
};
}

#endif HALF_EDGE_STRUCT_H
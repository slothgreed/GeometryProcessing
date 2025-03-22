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

class HalfEdgeStruct
{
public:

	struct IndexedFace
	{
		std::array<int, 3> position;
		std::array<int, 3> edge;
	};

	struct Edge
	{
		Vector3 begin;
		Vector3 end;
	};

	struct Face
	{
		Vector3 pos0;
		Vector3 pos1;
		Vector3 pos2;
		Vector3 Gravity() const
		{
			return (pos0 + pos1 + pos2) / 3.0f;
		}
	};

	typedef std::array<int,2> IndexedEdge;


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
	const Vector<Vector3>& GetPosition() const { return m_position; }
	const Vector<Vector3>& GetNormal();
	const Vector<Vector3>& GetNormal() const { assert(m_normal.size() == 0); return m_normal; }
	Edge GetEdge(int edgeIndex) const;
	
	IndexedEdge GetIndexedEdge(int edgeIndex) const;
	const IndexedFace& GetIndexedFace(int faceIndex) const;
	Face GetFace(int faceIndex) const;
	BDB CreateBDB() const;
	Vector<unsigned int> CreateIndexBufferData() const;
	Vector<unsigned int> CreateEdgeIndexBufferData();
	Vector3 CalcGravity(const IndexedFace& triangle) const;
	Vector3 CalcGravity(int faceIndex) const;
	Vector3 CalcFaceNormal(int faceIndex) const;

	Vector<int> GetAroundEdge(int positionIndex) const;
	Vector<unsigned int> GetAroundFace(const IndexedFace& triangle) const;
	Vector<int> GetAroundFaceFromPosition(int index) const;
	
	Vector3 GetNormal(int index);

	Vector<Vector3> ConvertVertexColorToFaceColor(const Vector<Vector3>& color) const;

private:

	void CreateNormal();
	void CreateFace();
	Vector<Vector3> m_normal;
	Vector<HalfEdgeStruct::IndexedFace> m_face;
	Vector<HalfEdge> m_halfEdge;
	Vector<Vector3> m_position;
	Vector<int> m_positionToEdge;
	Vector<int> m_faceToEdge;
};
}

#endif HALF_EDGE_STRUCT_H
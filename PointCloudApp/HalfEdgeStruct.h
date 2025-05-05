#ifndef HALF_EDGE_STRUCT_H
#define HALF_EDGE_STRUCT_H
#include "BDB.h"
#include "Parameter.h"
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

	enum VertexParameter
	{
		None,
		HeatValue,
		VertexArea,
		SDF,
		Num
	};


	static String ToString(VertexParameter param);
	static const char* const* GetVertexParameterString();
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
	size_t GetVertexNum() const { return m_position.size(); }
	size_t GetEdgeNum() const { return m_halfEdge.size(); }
	const Vector3& GetVertex(int index) const { return m_position[index]; }
	const Vector<Vector3>& GetVertex() const { return m_position; }
	const Vector<Vector3>& GetNormal();
	const Vector<Vector3>& GetNormal() const { assert(m_parameter.vertexNormal.size() == 0); return m_parameter.vertexNormal; }
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
	float CalcFaceArea(int faceIndex) const;
	Vector<int> GetAroundEdge(int positionIndex) const;
	Vector<unsigned int> GetAroundFace(const IndexedFace& triangle) const;
	Vector<int> GetAroundFaceFromPosition(int index) const;
	const HalfEdge& GetHalfEdge(int edgeIndex) const { return m_halfEdge[edgeIndex]; }
	Vector3 GetNormal(int index);

	float CalcCotangent(int edgeIndex) const;
	Vector<Vector3> ConvertVertexColorToFaceColor(const Vector<Vector3>& color) const;

	void CreateVertexArea();
	float CalcVertexArea(int position) const;
	float GetVertexArea(int position) const;
	const Vector<float>& GetVertexArea() const { return m_parameter.vertexArea; }

	void CreateCotangentLaplasian();
	void CreateHeatMethod(float timeStep, int position);
	const Vector<float>& GetHeatValue() const { return m_parameter.heatValue; }
	void AddVertexOnFace(int faceIndex);
	void AddVertexOnEdge(int edgeIndex);
private:

	void CreateNormal();
	void CreateFace();
	Vector<HalfEdgeStruct::IndexedFace> m_face;
	Vector<HalfEdge> m_halfEdge;
	Vector<Vector3> m_position;
	Vector<int> m_positionToEdge;
	Vector<int> m_faceToEdge;

	struct Parameter
	{
		Eigen::SparseMatrix<float> cotangent;
		Vector<float> heatValue;
		Vector<float> vertexArea;
		Vector<Vector3> vertexNormal;
	};

	Parameter m_parameter;
};
}

#endif HALF_EDGE_STRUCT_H
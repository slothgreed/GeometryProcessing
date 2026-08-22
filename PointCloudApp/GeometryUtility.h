#ifndef GEOMETRY_UTILITY_H
#define GEOMETRY_UTILITY_H
#include "BDB.h"
namespace KI
{
typedef std::pair<int, unsigned int> TriangleToMorton;
class MortonCode
{
public:
	MortonCode() {};
	~MortonCode() {};

	struct Data
	{
		Data()
			: triangleIndex(-1)
			, morton(-1)
		{
		}
		BDB box;
		int triangleIndex;
		int morton;
	};


	void Create(const Vector<Vector3>& triangle, const Vector<unsigned int>& trinagle, const BDB& box, int resolute = 1024);
	static unsigned int To(unsigned int x);
	static unsigned int To(const Vector3& x);
	static Vector3 ToColor(unsigned int morton);
	const Vector<Data>& Get() const { return m_mortons; }
private:
	Vector<Data> m_mortons;
};


class HalfEdgeStruct;
class HalfEdgeNode;
class Mesh;
class MeshAlgorithm
{
public:
	MeshAlgorithm() {};
	~MeshAlgorithm() {};

	// 既存の点群を保持したうえでサンプリングする。
	// meshPoints == sampleNum: そのままreturn
	// meshPoints < sampleNum: 足りない分だけ免状に任意にサンプリングする。
	// meshPoints > sampleNum: 既存の点をランダムに削減する。(本来は特徴量に応じて削減するべき)
	static Vector<Vector3> CreateSampleOnFace(const Mesh& mesh, int sampleNum);
	// 既存の点群は無視してサンプリングする。
	static Vector<Vector3> CreatePoissonSampleOnFace(const HalfEdgeStruct& halfEdge);
	static Vector<Vector3> CreatePoissonSampleVolume(HalfEdgeNode& halfEdge);
	static Vector<Vector3> CreateKruskulMST(const HalfEdgeStruct& halfEdge, Vector<Vector3>& seam, float weight);
private:

};

class GeometryUtility
{
public:
	GeometryUtility() {};
	~GeometryUtility() = default;

	static void CreateTangentBasis(const Vector3& value, Vector3& t1, Vector3& t2);
	static float CalcCotangent(const Vector3& p0, const Vector3& p1, const Vector3& p2);
	static bool IsCCW(const Vector2& screen0, const Vector2& screen1, const Vector2& screen2);
	static float CalcArea(const Vector3& pos0, const Vector3& pos1, const Vector3& pos2);
	static float CalcArea(const Vector2& screen0, const Vector2& screen1, const Vector2& screen2);
private:

};


}



#endif GEOMETRY_UTILITY_H

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
class MeshAlgorithm
{
public:
	MeshAlgorithm() {};
	~MeshAlgorithm() {};

	static Vector<Vector3> CreatePoissonSampleOnFace(const HalfEdgeStruct& halfEdge);
	static Vector<Vector3> CreatePoissonSampleVolume(HalfEdgeNode& halfEdge);
	static Vector<Vector3> CreateKruskulMST(const HalfEdgeStruct& halfEdge, Vector<Vector3>& seam, float weight);
private:

};

class GeometryUtility
{
public:
	GeometryUtility();
	~GeometryUtility();

	static void CreateTangentBasis(const Vector3& value, Vector3& t1, Vector3& t2);
	static float CalcCotangent(const Vector3& p0, const Vector3& p1, const Vector3& p2);
private:

};


}



#endif GEOMETRY_UTILITY_H

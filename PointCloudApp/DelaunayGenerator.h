#ifndef DELAUNAY_GENERATOR
#define DELAUNAY_GENERATOR
#include "IAlgorithm.h"
namespace KI
{

class PointCloudNode;
class DelaunayGenerator : public IAlgorithm
{
public:
	DelaunayGenerator(PointCloudNode* pPointCloud);
	~DelaunayGenerator();

	ALGORITHM_TYPE GetType() { return ALGORITHM_DELAUNAY; }
	virtual void Execute() {};
	Vector<unsigned int> Execute2D(int iterate);
	void ShowUI();

private:

	struct Circumscribe
	{
		Circumscribe() :radius(0) {}
		float radius;
		Vector3 center;
	};

	struct Triangle
	{
		Triangle() {}
		Triangle(const Vector3& p0, const Vector3& p1, const Vector3& p2) : pos0(p0), pos1(p1), pos2(p2) {}
		
		Vector3 pos0;
		Vector3 pos1;
		Vector3 pos2;
	};

	static const int HUGE_POS0 = -1;
	static const int HUGE_POS1 = -2;
	static const int HUGE_POS2 = -3;

	struct IndexedTriangle
	{
		friend class DelaunayGenerator;
		IndexedTriangle() : pos0(-1), pos1(-1), pos2(-1) {}
		IndexedTriangle(int p0, int p1, int p2) : pos0(p0), pos1(p1), pos2(p2) {}
		int pos0;
		int pos1;
		int pos2;

		Triangle Convert(DelaunayGenerator* pGen) const
		{
			Triangle tri;
			if (pos0 == HUGE_POS0) { tri.pos0 = pGen->m_HugeTriangle.pos0; }
			else if (pos0 == HUGE_POS1) { tri.pos0 = pGen->m_HugeTriangle.pos1; }
			else if (pos0 == HUGE_POS2) { tri.pos0 = pGen->m_HugeTriangle.pos2; }
			else { tri.pos0 = pGen->m_positions[pos0]; }

			if (pos1 == HUGE_POS0) { tri.pos1 = pGen->m_HugeTriangle.pos0; }
			else if (pos1 == HUGE_POS1) { tri.pos1 = pGen->m_HugeTriangle.pos1; }
			else if (pos1 == HUGE_POS2) { tri.pos1 = pGen->m_HugeTriangle.pos2; }
			else { tri.pos1 = pGen->m_positions[pos1]; }


			if (pos2 == HUGE_POS0) { tri.pos2 = pGen->m_HugeTriangle.pos0; }
			else if (pos2 == HUGE_POS1) { tri.pos2 = pGen->m_HugeTriangle.pos1; }
			else if (pos2 == HUGE_POS2) { tri.pos2 = pGen->m_HugeTriangle.pos2; }
			else { tri.pos2 = pGen->m_positions[pos2]; }
			return tri;
		}

		bool IsSame(const IndexedTriangle& tri) const
		{
			if (pos0 == tri.pos0 && pos1 == tri.pos1 && pos2 == tri.pos2) { return true; }
			if (pos0 == tri.pos0 && pos1 == tri.pos2 && pos2 == tri.pos1) { return true; }
			
			if (pos0 == tri.pos1 && pos1 == tri.pos0 && pos2 == tri.pos2) { return true; }
			if (pos0 == tri.pos1 && pos1 == tri.pos2 && pos2 == tri.pos0) { return true; }
			
			if (pos0 == tri.pos2 && pos1 == tri.pos1 && pos2 == tri.pos0) { return true; }
			if (pos0 == tri.pos2 && pos1 == tri.pos0 && pos2 == tri.pos1) { return true; }

			return false;
		}

		bool HasHuge() const
		{
			return
				pos0 == HUGE_POS0 || pos0 == HUGE_POS1 || pos0 == HUGE_POS2 ||
				pos1 == HUGE_POS0 || pos1 == HUGE_POS1 || pos1 == HUGE_POS2 ||
				pos2 == HUGE_POS0 || pos2 == HUGE_POS1 || pos2 == HUGE_POS2;
		}
	};

	void Division(int index);
	bool InnerByCircle(const DelaunayGenerator::Circumscribe& circle, const Vector3& point);
	Circumscribe CalcCircumscribedCircle(const Triangle& triangle);
	Circumscribe CalcCircumscribedCircle(const IndexedTriangle& triangle);
	Triangle CreateHugeTriangle2D();
	void RemoveHugeTriangle();
	
	
	PointCloudNode* m_pPointCloud;
	const Vector<Vector3>& m_positions;
	Triangle m_HugeTriangle;
	List<IndexedTriangle> m_Delaunay;
};

}

#endif DELAUNAY_GENERATOR

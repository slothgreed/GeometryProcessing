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
	Vector<unsigned int> Execute2D(const Vector<Vector3>& points);
	void ShowUI();

private:

	struct Circumscribe
	{
		float radius;
		Vector3 center;
	};

	struct Triangle
	{
		Vector3 pos0;
		Vector3 pos1;
		Vector3 pos2;
	};

	PointCloudNode* m_pPointCloud;
	Circumscribe CalcCircumscribedCircle(const Triangle& triangle);
	Triangle CreateHugeTriangle2D(const Vector<Vector3>& points);
	Triangle m_HugeTriangle;
};

}

#endif DELAUNAY_GENERATOR

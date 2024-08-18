#ifndef DELAUNAY_GENERATOR
#define DELAUNAY_GENERATOR

namespace KI
{

class DelaunayGenerator
{
public:
	DelaunayGenerator();
	~DelaunayGenerator();

	Vector<unsigned int> Execute2D(const Vector<Vector3>& points);
	void ShowUI();

private:
	struct Triangle
	{
		Vector3 pos0;
		Vector3 pos1;
		Vector3 pos2;
	};
	Triangle CreateHugeTriangle2D(const Vector<Vector3>& points);
	Vector<Vector3> m_HugeTriangle;
};

}

#endif DELAUNAY_GENERATOR

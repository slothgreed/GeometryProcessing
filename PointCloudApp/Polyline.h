#ifndef POLYLINE_H
#define POLYLINE_H

namespace KI
{
class Polyline
{
public:
	Polyline(Vector<Vector3>&& points);
	Polyline() {};
	~Polyline() {};

	int Num() const { return m_points.size(); }
	Vector3 Last() const { return m_points[m_points.size() - 1]; }
	void Add(const Vector3& point) { m_points.push_back(point); }
	const Vector<Vector3>& Get() const { return m_points; }
	Vector<unsigned int> CreateTriangles() const;
	Vector<Vector3> CreateTrianglePoints() const;
	Vector<Vector3> CreateLinePoints() const;
	Vector<Vector3> Rotate2D() const;
	Vector3 GetNormal() const;
	bool IsPlane() const;
	bool IsNormalZPlus() const;
	bool IsNormalZMinus() const;
private:
	Vector<Vector3> m_points;
};

}

#endif POLYLINE_H
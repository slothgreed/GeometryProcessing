#ifndef POLYLINE_H
#define POLYLINE_H

namespace KI
{
class Polyline
{
public:
	Polyline() {};
	Polyline(Vector<Vector3>&& points)
		:m_points(std::move(points)) {};
	~Polyline() {};

	int Num() const { return m_points.size(); }
	Vector3 Last() const { return m_points[m_points.size() - 1]; }
	void Add(const Vector3& point) { m_points.push_back(point); }
private:
	Vector<Vector3> m_points;
};

}

#endif POLYLINE_H
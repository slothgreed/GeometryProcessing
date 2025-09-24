#ifndef POLYLINE_H
#define POLYLINE_H

namespace KI
{
class Polyline
{
public:
	enum class Hint
	{
		None,
		Arbitrary,
		Circle
	};
	Polyline(Vector<Vector3>&& points);
	Polyline() : m_hint(Hint::None) {};
	~Polyline() {};


	int Num() const { return m_points.size(); }
	Vector3 Last() const { return m_points[m_points.size() - 1]; }
	void Add(const Vector3& point) { m_points.push_back(point); }
	void Add(Vector<Vector3>&& point);
	void Add(Polyline&& polyline);
	void AddCircle(Polyline&& polyline);
	const Vector<Vector3>& Get() const { return m_points; }
	Vector<unsigned int> CreateTriangles() const;
	Vector<Vector3> CreateTrianglePoints(bool orient) const;
	Vector<Vector3> CreateLinePoints() const;
	Vector3 GetNormal() const;
	Vector3 GetCenter() const;
	bool IsPlane() const;
	static Vector<Vector3> CraeteDelaunay(const Polyline& target, const Polyline& inner);
private:
	Vector<Vector3> CreateUnique() const;
	Vector<Vector3> m_points;
	Hint m_hint;
};

}

#endif POLYLINE_H
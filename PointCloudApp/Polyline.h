#ifndef POLYLINE_H
#define POLYLINE_H

namespace KI
{
class UVConverter
{
	virtual Vector2 toUV(const Vector3& xyz) = 0;
	virtual Vector3 toXYZ(const Vector2& uv) = 0;
};
class Polyline
{
public:
	enum class Hint
	{
		None,
		Arbitrary,
		Circle,
		LineLoop,
		Lines
	};
	Polyline(Vector<Vector3>&& points);
	Polyline(Vector<Vector3>&& points, Hint hint);
	Polyline(Vector<Vector3>&& points, Vector<unsigned int> uInt, Hint hint);
	Polyline() : m_hint(Hint::None) {};
	~Polyline() {};


	int Num() const { return m_points.size(); }
	Vector3 Last() const { return m_points[m_points.size() - 1]; }
	//void Add(const Vector3& point) { m_points.push_back(point); }
	void AddLoop(Polyline&& point);
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
	Vector<unsigned int> m_indexs;
	UVConverter* m_pUVConverter;
	Hint m_hint;
};

class PolylineList
{
public:
	PolylineList();
	~PolylineList();

	void Add(Polyline&& polyline) { m_polylines.push_back(std::move(polyline)); }
private:
	Vector<Polyline> m_polylines;
};


}

#endif POLYLINE_H
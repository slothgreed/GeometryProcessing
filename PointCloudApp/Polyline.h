#ifndef POLYLINE_H
#define POLYLINE_H
#include "BDB.h"
namespace KI
{
class IUVConverter
{
public:
	virtual Vector2 toUV(const Vector3& xyz) const = 0;
	virtual Vector3 toXYZ(const Vector2& uv) const = 0;
};
class Polyline
{
public:

	enum class DrawType
	{
		Lines = GL_LINES,
		LineLoop = GL_LINE_LOOP,
		LineStrip = GL_LINE_STRIP
	};

	Polyline(Vector<Vector3>&& points);
	Polyline(Vector<Vector3>&& points, DrawType drawType);
	Polyline(Vector<Vector3>&& points, Vector<UInt>&& uInt, DrawType drawType);
	Polyline() {};
	~Polyline() {};


	int PointNum() const { return m_points.size(); }
	int LineNum() const;
	Vector3 Last() const { return m_points[m_points.size() - 1]; }
	void Add(const Polyline& point);
	void Add(Polyline&& point);

	const Vector<Vector3>& GetPoints() const { return m_points; }
	const Vector<UInt>& GetIndexs() const { return m_indexs; }

	void Set(int index, const Vector3& data) { m_points[index] = data; }
	GLuint GetDrawType() const { return (GLuint)m_drawType; }
	Vector<Vector3> CreateTriangleArray() const;
	Vector<Vector3> CreateTriangleLine() const;
	Vector<UInt> CreateTriangles() const;
	Vector<Vector3> CreateTrianglePoints(bool orient) const;
	Vector<Vector3> CreateLinePoints() const;
	Vector3 GetNormal() const;
	Vector3 GetCenter() const;
	void Reverse();
	bool IsPlane() const;
	static Vector<Vector3> CraeteDelaunay(const Polyline& target, const Polyline& inner);
	Polyline CreateSmooth() const;
	static BDB CreateBDB(const Polyline& polyline);
private:
	Vector<Vector3> CreateUnique() const;
	Vector<Vector3> m_points;
	Vector<UInt> m_indexs;
	DrawType m_drawType = DrawType::LineLoop;
};

struct PolylineList
{
	PolylineList() {};
	PolylineList(Polyline&& poly) { m_polylines.push_back(std::move(poly)); }
	void Add(Polyline&& poly) { m_polylines.push_back(std::move(poly)); }
	void Add(PolylineList&& poly);
	Polyline Merge() const;
	size_t Num() const { return m_polylines.size(); }
private:
	Vector<Polyline> m_polylines;
};
}

#endif POLYLINE_H
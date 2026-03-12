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
	enum class Hint
	{
		None,
		Arbitrary,
		Circle,
	};

	enum class DrawType
	{
		Lines = GL_LINES,
		LineLoop = GL_LINE_LOOP,
		LineStrip = GL_LINE_STRIP
	};

	Polyline(Vector<Vector3>&& points);
	Polyline(Vector<Vector3>&& points, DrawType drawType);
	Polyline(Vector<Vector3>&& points, Vector<UInt>&& uInt, DrawType drawType);
	Polyline() : m_hint(Hint::None) {};
	~Polyline() {};


	int PointNum() const { return m_points.size(); }
	int LineNum() const;
	Vector3 Last() const { return m_points[m_points.size() - 1]; }
	//void Add(const Vector3& point) { m_points.push_back(point); }
	void AddLoop(const Polyline& point);
	void AddLoop(Polyline&& point);
	void AddCircle(Polyline&& polyline);

	const Vector<Vector3>& Get() const { return m_points; }
	const Vector<UInt>& GetIndex() const { return m_indexs; }

	void Set(int index, const Vector3& data) { m_points[index] = data; }
	void SetUVConverter(const Shared<IUVConverter>& pPtr) { m_pUVConverter = pPtr; }
	GLuint GetDrawType() const { return (GLuint)m_drawType; }
	Hint GetHint() const { return m_hint; }
	Vector<Vector3> CreateTriangleArray() const;
	Vector<Vector3> CreateTriangleLine() const;
	Vector<UInt> CreateTriangles() const;
	Vector<Vector3> CreateTrianglePoints(bool orient) const;
	Vector<Vector3> CreateLinePoints() const;
	Vector3 GetNormal() const;
	Vector3 GetCenter() const;
	bool IsPlane() const;
	static Vector<Vector3> CraeteDelaunay(const Polyline& target, const Polyline& inner);
	Polyline CreateSmooth() const;
	static BDB CreateBDB(const Polyline& polyline);
	static Polyline ToUV(const Polyline& polyline);
	static Polyline ToXYZ(const Polyline& polyline);

private:
	Vector<Vector3> CreateUnique() const;
	Vector<Vector3> m_points;
	Vector<UInt> m_indexs;
	Shared<IUVConverter> m_pUVConverter = nullptr;
	Hint m_hint = Hint::None;
	DrawType m_drawType = DrawType::Lines;
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
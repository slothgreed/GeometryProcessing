#ifndef KI_MATH_H
#define KI_MATH_H
#include "BDB.h"
namespace KI
{
class MathHelper
{
public:
	MathHelper() {};
	~MathHelper() {};

	static void SphericalToCartesian(float radius, float x, float y, Vector3& result);
	static void CartesianToSpherical(const Vector3& position, float& radian, float& theta, float& phi);
	static float CalcTriangleArea(const Vector3& v1, const Vector3& v2, const Vector3& v3); // counter cloclk wise
	static Vector3 CalcNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3); // counter cloclk wise
	static float CalcRadian(const Vector3& v1, const Vector3& v2);
	static Matrix4x4 CreateRotateMatrix(const Vector3& source, const Vector3& target);
	static float Round(float value, float eps = 0.0001f);
	static float CramesDet(const Vector3& a, const Vector3& b, const Vector3& c);
	static float ToRadian(float angle);
	static float ToAngle(float rad);
	static Vector4 WorldToScreenPos(const Matrix4x4& proj, const Matrix4x4& view, const Matrix4x4& model, const Vector3& value);
	static bool InPolyline(const Vector<Vector3>& polyline, const Vector3& point, bool isLoop);
	static void CalcClasterPoints(const Vector<Vector3>& points, const Vector3& begin, const Vector3& end, Vector<Vector3>& left, Vector<Vector3>& right);
	static bool IsLoop(const Vector<Vector3>& polyline);
	static bool IsSame(float v1, float v2);
	static bool IsSame(const Vector3& v1, const Vector3& v2);
	static bool IsZero(float v1);
	static bool IsOne(float v1);
	static bool IsZPlus(const Vector3& value);
	static bool IsZMinus(const Vector3& value);
	static bool IsZ(const Vector3& value);
	static Vector<Vector3> To2D(const Vector<Vector3>& point);
	static Vector<Vector3> Rotate(const Vector<Vector3>& point, const Matrix4x4& matrix);
	static Matrix4x4 CreateZAxisMatrix(const Vector3& normal);
private:

};

class Intersect
{
public:
	Intersect() {}
	~Intersect() {}

	struct Result
	{
		Result() :success(false), distance(0) {}
		Result(float v) :success(true), distance(v) {}
		Result(float v, const Vector3& p) :success(true), distance(v), position(p) {}
		bool success;
		Vector3 position;
		float distance;
	};

	// point to triangle
	static Result PointToTriangle(const Vector3& p, const Vector3& t0, const Vector3& t1, const Vector3& t2);
	static Result PointToEdge(const glm::vec3& P, const glm::vec3& X, const glm::vec3& Y);
	static Result PointToBox(const glm::vec3& P, const BDB& bdb, bool innerDist);

private:

};



class Ray
{
public:
	Ray() : m_origin(Vector3(0)), m_direction(Vector3(0)) {};
	Ray(const Vector3& origin, const Vector3& direction) { Set(origin, direction); };
	~Ray() {};

	void Set(const Vector3& origin, const Vector3& direction) { m_origin = origin; m_direction = direction; }
	const Vector3& Origin() const { return m_origin; };
	const Vector3& Direction() const { return m_direction; };

	struct IntersectResult
	{
		IntersectResult()
			: success(false)
			, distance(0)
		{
		};

		IntersectResult(const Vector3& p, float d)
			:success(true)
			, position(p)
			, distance(d)
		{
		};

		static IntersectResult CreateFailed()
		{
			return IntersectResult();
		}
		static IntersectResult CreateSuccess()
		{
			IntersectResult result;
			result.success = true;
			return result;
		}

		bool success;
		Vector3 position;
		float distance;
	};


	// orient = true : å¸Ç´Ççló∂Ç∑ÇÈÅB
	IntersectResult Intersect(const Vector3& p0, const Vector3& p1, const Vector3& p2, bool orient) const;
	IntersectResult Intersect(const BDB& bdb) const;
private:
	Vector3 m_origin;
	Vector3 m_direction;
};
}



#endif KI_MATH_H
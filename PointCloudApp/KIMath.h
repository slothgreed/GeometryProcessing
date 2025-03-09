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
	static void CalcNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3, Vector3& normal); // counter cloclk wise
	static float CalcRadian(const Vector3& v1, const Vector3& v2);
	static Matrix4x4 CreateRotateMatrix(const Vector3& source, const Vector3& target);
	static float Round(float value, float eps = 0.0001f);
	static float CramesDet(const Vector3& a, const Vector3& b, const Vector3& c);
	static float ToRadian(float angle);
	static float ToAngle(float rad);
	static Vector4 WorldToScreenPos(const Matrix4x4& proj, const Matrix4x4& view, const Matrix4x4& model, const Vector3& value);

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
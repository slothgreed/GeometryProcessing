#include "KIMath.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
namespace KI
{
void MathHelper::SphericalToCartesian(float radius, float x, float y, Vector3& result)
{
	result.x = radius * sin(y) * cos(x);
	result.y = radius * cos(y);
	result.z = radius * sin(y) * sin(x);
}

void MathHelper::CartesianToSpherical(const Vector3& position, float& radian, float& theta, float& phi)
{
	radian = glm::length(position);
	phi = glm::acos(position.z / radian);
	theta = glm::atan(position.y / position.x);
}

float MathHelper::CalcTriangleArea(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	Vector3 vector1 = v2 - v1;
	Vector3 vector2 = v3 - v1;

	Vector3 exterior = cross(vector1, vector2);

	float area = length(exterior) / 2;

	return area;
}

void MathHelper::CalcNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3, Vector3& normal)
{
	Vector3 vector1 = normalize(v2 - v1);
	Vector3 vector2 = normalize(v3 - v1);

	normal = cross(vector1,vector2);
}

float MathHelper::CalcRadian(const Vector3& v1, const Vector3& v2) 
{
	Vector3 vector1 = normalize(v1);
	Vector3 vector2 = normalize(v2);

	float rad = dot(vector1, vector2);

	return acos(rad);
}

float MathHelper::ToRadian(float angle)
{
	return angle * glm::pi<float>() / 180;
}

float MathHelper::ToAngle(float rad)
{
	return rad * 180 / glm::pi<float>();
}

float MathHelper::CramesDet(const Vector3& a, const Vector3& b, const Vector3& c)
{
	return (
		  (a.x * b.y * c.z)
		+ (a.y * b.z * c.x)
		+ (a.z * b.x * c.y)
		- (a.x * b.z * c.y)
		- (a.y * b.x * c.z)
		- (a.z * b.y * c.x));
}

Vector4 MathHelper::WorldToScreenPos(const Matrix4x4& proj, const Matrix4x4& view, const Matrix4x4& model, const Vector3& value)
{
	vec4  result = proj * view * model * vec4(value.x, value.y, value.z, 1.0);

	return result / result.w;
}

float MathHelper::Round(float value, float eps)
{
	if (1.0f - value < eps) {
		return 1.0f;
	}
	else if (1.0 + value < eps) {
		return -1.0f;
	}

	return value;
}

Matrix4x4 MathHelper::CreateRotateMatrix(const Vector3& source, const Vector3& target)
{
	float dot = Round(glm::dot(source, target));
	if (dot == 1.0f) {
		return Matrix4x4(1);
	}

	if (dot == -1.0f) {
		if (source == Vector3(0, 0, 1)) {
			return glm::rotate(180.0f, Vector3(0, 1, 0));
		}
		else {
			return glm::rotate(180.0f, Vector3(0, 0, 1));
		}
	}


	Vector3 cross1 = glm::cross(source, target);
	cross1 = glm::normalize(cross1);
	quat axis = angleAxis(glm::angle(source, target), cross1);
	Vector3 test = glm::toMat4(axis) * vec4(source, 1.0);
	if (!(std::abs(test.x - target.x) < 0.001 &&
		std::abs(test.y - target.y) < 0.001 &&
		std::abs(test.z - target.z) < 0.001))
	{
		assert(0);
	}

	return glm::toMat4(axis);

}
// refer : https://shikousakugo.wordpress.com/2012/06/27/ray-intersection-2/
// Tomas Mollerの交差判定,　クラメルの公式利用
Ray::IntersectResult Ray::Intersect(const Vector3& p0, const Vector3& p1, const Vector3& p2, bool orient)
{
	const float epsilon = 1e-6f; // 精度の閾値
	if (glm::distance(m_origin, p0) < epsilon || 
		glm::distance(m_origin, p1) < epsilon || 
		glm::distance(m_origin, p2) < epsilon) {
		return Ray::IntersectResult();
	}

	glm::vec3 AB = p1 - p0;
	glm::vec3 AC = p2 - p0;
	glm::vec3 P = glm::cross(m_direction, AC);

	float det = glm::dot(AB, P);
	// 面の向きを考慮しない場合はfabsを外す。
	if (orient) {
		if (det < epsilon) {
			return IntersectResult(); // 平行
		}
	} else {
		if (fabs(det) < epsilon) {
			return IntersectResult(); // 平行
		}
	}

	float invDet = 1.0f / det;
	glm::vec3 T = m_origin - p0;;
	float u = glm::dot(T, P) * invDet;
	if (u < 0.0f || u > 1.0f) {
		return IntersectResult(); // 三角形の外側
	}

	glm::vec3 Q = glm::cross(T, AB);
	float v = glm::dot(m_direction, Q) * invDet;
	if (v < 0.0f || u + v > 1.0f) {
		return IntersectResult(); // 三角形の外側
	}

	float t = glm::dot(AC, Q) * invDet; // 交差点までの距離
	if (t <= 0) { return IntersectResult(); }

	return IntersectResult(m_origin + m_direction * t, t);
}
}
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

Vector3 MathHelper::CalcNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	Vector3 vector1 = normalize(v2 - v1);
	Vector3 vector2 = normalize(v3 - v1);

	return cross(vector1,vector2);
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


Intersect::Result Intersect::PointToBox(const glm::vec3& P, const BDB& bdb, bool innerDist)
{
	// AABB 内部にいる場合は、表面までの最短距離を計算
	glm::vec3 C; // AABB の最近点
	C.x = std::max(bdb.Min().x, std::min(P.x, bdb.Max().x));
	C.y = std::max(bdb.Min().y, std::min(P.y, bdb.Max().y));
	C.z = std::max(bdb.Min().z, std::min(P.z, bdb.Max().z));

	if (P.x >= bdb.Min().x && P.x <= bdb.Max().x &&
		P.y >= bdb.Min().y && P.y <= bdb.Max().y &&
		P.z >= bdb.Min().z && P.z <= bdb.Max().z) {

		if (!innerDist) { return Intersect::Result(0); }
		// AABB の内部にいる場合、最短の面上の点を求める
		float dx = std::min(P.x - bdb.Min().x, bdb.Max().x - P.x);
		float dy = std::min(P.y - bdb.Min().y, bdb.Max().y - P.y);
		float dz = std::min(P.z - bdb.Min().z, bdb.Max().z - P.z);
		float minDist = std::min({ dx, dy, dz });

		// どの面に最も近いか判定し、C を修正
		if (minDist == dx) {
			C.x = (P.x - bdb.Min().x < bdb.Max().x - P.x) ? bdb.Min().x : bdb.Max().x;
		} else if (minDist == dy) {
			C.y = (P.y - bdb.Min().y < bdb.Max().y - P.y) ? bdb.Min().y : bdb.Max().y;
		} else {
			C.z = (P.z - bdb.Min().z < bdb.Max().z - P.z) ? bdb.Min().z : bdb.Max().z;
		}
	}

	return Intersect::Result(glm::length(P - C));
}

Intersect::Result Intersect::PointToEdge(const glm::vec3& P, const glm::vec3& X, const glm::vec3& Y)
{
	glm::vec3 XY = Y - X;
	glm::vec3 XP = P - X;
	float t = glm::dot(XP, XY) / glm::dot(XY, XY);
	t = glm::clamp(t, 0.0f, 1.0f);
	glm::vec3 Q_edge = X + t * XY;
	return Intersect::Result(glm::length(P - Q_edge), Q_edge);
};

Intersect::Result Intersect::PointToTriangle(const Vector3& P, const Vector3& A, const Vector3& B, const Vector3& C)
{
	// 三角形の法線を求める
	glm::vec3 AB = B - A;
	glm::vec3 AC = C - A;
	glm::vec3 N = glm::normalize(glm::cross(AB, AC));

	// P から三角形の平面への垂線の足を求める
	float d = glm::dot(P - A, N);
	glm::vec3 Q_face = P - d * N; // 垂線の足

	// Q_face が三角形の内部にあるか判定（バリセントリック座標を使用）
	glm::vec3 v0 = C - A, v1 = B - A, v2 = Q_face - A;
	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	if (std::abs(denom) < 1e-6f) {
		return Intersect::Result(glm::length(P - A), A);
	}
	float u = (d11 * d20 - d01 * d21) / denom;
	float v = (d00 * d21 - d01 * d20) / denom;

	if (u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f) {
		// Q_face が三角形の内部にある場合
		return Intersect::Result(std::abs(d), Q_face);
	} else {
		auto edge0 = PointToEdge(P, A, B);
		auto edge1 = PointToEdge(P, B, C);
		auto edge2 = PointToEdge(P, C, A);

		// 最短距離を持つものを選択
		return std::min({ edge0, edge1, edge2 }, [](const Intersect::Result& a, const Intersect::Result& b)	{ return a.distance < b.distance; });
	}
}
// refer : https://shikousakugo.wordpress.com/2012/06/27/ray-intersection-2/
// Tomas Mollerの交差判定,　クラメルの公式利用
Ray::IntersectResult Ray::Intersect(const Vector3& p0, const Vector3& p1, const Vector3& p2, bool orient) const
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
Ray::IntersectResult Ray::Intersect(const BDB& bdb) const
{
	auto invDir = 1.0f / m_direction;
	auto t0s = (bdb.Min() - m_origin) * invDir;
	auto t1s = (bdb.Max() - m_origin) * invDir;

	auto tMinVec = glm::min(t0s, t1s);
	auto tMaxVec = glm::max(t0s, t1s);

	float tMin = glm::max(glm::max(tMinVec.x, tMinVec.y), tMinVec.z);
	float tMax = glm::min(glm::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

	if (tMax >= tMin && tMax > 0.0f) {
		return Ray::IntersectResult::CreateSuccess();
	}

	return Ray::IntersectResult::CreateFailed();

}

}
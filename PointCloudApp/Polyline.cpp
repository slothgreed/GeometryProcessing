#include "Polyline.h"
#include "DelaunayGenerator.h"
namespace KI
{

Polyline::Polyline(Vector<Vector3>&& points)
	:m_points(std::move(points)) 
    ,m_hint(Hint::None)
{
    if (!IsPlane()) {
        assert(0);
    }
}

void Polyline::Add(Vector<Vector3>&& point)
{
    m_points.insert(m_points.end(), point.begin(), point.end()); 
    m_hint = Hint::Arbitrary;
}
void Polyline::AddCircle(Vector<Vector3>&& circle)
{
    m_points.insert(m_points.end(), circle.begin(), circle.end());
    if (m_hint == Hint::None) {
        m_hint = Hint::Circle;
    }
}

Vector3 Polyline::GetCenter() const
{
    Vector3 center = Vector3(0);
    for (const auto& p : m_points) {
        center += p;
    }

    return center /= m_points.size();
}
Vector<unsigned int> Polyline::CreateTriangles() const
{
    auto normal = GetNormal();
    DelaunayGenerator delaunay;
    if (IsNormalZPlus()) {
        delaunay.SetTarget(&m_points);
        return delaunay.Execute2D();
    } else if (IsNormalZMinus()) {
        delaunay.SetTarget(&m_points, false);
        return delaunay.Execute2D();
    } else {
        auto points = Rotate2D();
        delaunay.SetTarget(&points);
        return delaunay.Execute2D();
    }
}
Vector<Vector3> Polyline::CreateTrianglePoints(bool ccw) const
{
    Vector<Vector3> points;
    if (m_hint == Hint::Circle) {
        auto center = GetCenter();
        for (size_t i = 0; i < m_points.size() - 1; i++) {
            points.push_back(m_points[i]);
            if (ccw) {
                points.push_back(m_points[i + 1]);
                points.push_back(center);
            } else {
                points.push_back(center);
                points.push_back(m_points[i + 1]);
            }
        }

        points.push_back(m_points[m_points.size() - 1]);
        if (ccw) {
            points.push_back(m_points[0]);
            points.push_back(center);
        } else {
            points.push_back(center);
            points.push_back(m_points[0]);
        }
        return points;
    } else {
        auto indexs = CreateTriangles();
        points.resize(indexs.size());
        for (size_t i = 0; i < indexs.size(); i++) {
            points[i] = m_points[indexs[i]];
        }
    }


	return points;
}

Vector<Vector3> Polyline::CreateLinePoints() const
{
	Vector<Vector3> points;
	for (size_t i = 0; i < m_points.size() - 1; i++) {
		points.push_back(m_points[i]);
		points.push_back(m_points[i + 1]);
	}

	points.push_back(m_points[m_points.size() - 1]);
	points.push_back(m_points[0]);

	return points;
}
Vector3 Polyline::GetNormal() const
{
    if (m_points.size() < 3) return Vector3();
    // 平面の法線
    return glm::normalize(glm::cross(m_points[1] - m_points[0], m_points[2] - m_points[0]));

}

bool Polyline::IsNormalZPlus() const
{
    // 平面の法線
    // Z軸へ揃える回転
    float dotVal = glm::clamp(glm::dot(GetNormal(), Vector3(0, 0, 1)), -1.0f, 1.0f);
    if (fabs(dotVal - 1.0f) < 1e-6f) { return true; }
    return false;
}
bool Polyline::IsNormalZMinus() const
{
    float dotVal = -glm::clamp(glm::dot(GetNormal(), Vector3(0, 0, 1)), -1.0f, 1.0f);
    if (fabs(dotVal - 1.0f) < 1e-6f) { return true; }
    return false;
}
bool Polyline::IsPlane() const
{
    auto normal = GetNormal();

    // 全点が平面上か確認
    for (size_t i = 3; i < m_points.size(); i++) {
        float d = glm::dot(m_points[i] - m_points[0], normal);
        if (fabs(d) > 1e-6f) return false; // 平面上にない
    }

    return true;
}
Vector<Vector3> Polyline::Rotate2D() const
{
    Vector<Vector3> points;
    auto normal = GetNormal();

    // 平面の法線
    // Z軸へ揃える回転
    glm::vec3 target(0, 0, 1);
    float dotVal = glm::clamp(glm::dot(normal, target), -1.0f, 1.0f);

    // 法線がすでにZ軸と平行ならスキップ
    if (fabs(abs(dotVal) - 1.0f) < 1e-6f) { return m_points; }

    glm::vec3 axis = glm::normalize(glm::cross(normal, target));
    float angle = acos(dotVal);

    glm::mat4 R = glm::rotate(glm::mat4(1.0f), angle, axis);

    for (auto& p : m_points) {
        glm::vec4 v = R * glm::vec4(p, 1.0f);
        points.push_back(glm::vec3(v));
    }

    return points;
}
}
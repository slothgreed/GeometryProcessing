#include "Polyline.h"
#include "KIMath.h"
#include "DelaunayGenerator.h"
#include "Utility.h"
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
    STLUtil::Insert(m_points, point);
    m_points = CreateUnique();
    m_hint = Hint::Arbitrary;
}

void Polyline::Add(Polyline&& polyline)
{
    STLUtil::Insert(m_points, polyline.m_points);
    m_points = CreateUnique();
    m_hint = Hint::Arbitrary;
}
void Polyline::AddCircle(Polyline&& circle)
{
    STLUtil::Insert(m_points, circle.m_points);
    if (m_hint == Hint::None) {
        m_hint = Hint::Circle;
    }
}

Vector<Vector3> Polyline::CreateUnique() const
{
    Vector<Vector3> uniquePoints;
    uniquePoints.push_back(m_points[0]);
    for (int i = 0; i < m_points.size(); i++) {
        if (!MathHelper::IsSame(m_points[i],uniquePoints[uniquePoints.size() - 1])) {
            uniquePoints.push_back(m_points[i]);
        }
    }
    return uniquePoints;
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
    if (MathHelper::IsZ(normal)) {
        delaunay.SetTarget(&m_points);
        return delaunay.Execute2D();
    } else {
        auto points = MathHelper::To2D(m_points);
        delaunay.SetTarget(&points);
        return delaunay.Execute2D();
    }
}

Vector<Vector3> Polyline::CraeteDelaunay(const Polyline& target, const Polyline& inner)
{
    DelaunayGenerator delaunay;
    delaunay.SetTarget(&target.Get());
    
    if (inner.Get().size() != 0) {
        auto targetNormal = target.GetNormal();
        auto innerNormal = inner.GetNormal();
        if (!MathHelper::IsOne(fabs(glm::dot(targetNormal, innerNormal)))) {
            return Vector<Vector3>();
        }
        delaunay.AddInner(&inner.Get());
    }
    return delaunay.Execute2DTriangles();
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
    if (m_points.size() == 0) { return Vector<Vector3>(); }
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
    // ���ʂ̖@��
    return glm::normalize(glm::cross(m_points[1] - m_points[0], m_points[2] - m_points[0]));

}

bool Polyline::IsPlane() const
{
    auto normal = GetNormal();

    // �S�_�����ʏォ�m�F
    for (size_t i = 3; i < m_points.size(); i++) {
        float d = glm::dot(m_points[i] - m_points[0], normal);
        if (fabs(d) > 1e-6f) return false; // ���ʏ�ɂȂ�
    }

    return true;
}

}
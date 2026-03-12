#include "Polyline.h"
#include "KIMath.h"
#include "DelaunayGenerator.h"
#include "Utility.h"
namespace KI
{

Polyline::Polyline(Vector<Vector3>&& points)
	: m_points(std::move(points)) 
    , m_pUVConverter(nullptr)
    , m_hint(Hint::None)
{
}

Polyline::Polyline(Vector<Vector3>&& points, DrawType drawType)
    : m_points(std::move(points))
    , m_pUVConverter(nullptr)
    , m_drawType(drawType)
{
}

Polyline::Polyline(Vector<Vector3>&& points, Vector<unsigned int>&& uInt, DrawType drawType)
    : m_points(std::move(points))
    , m_pUVConverter(nullptr)
    , m_indexs(uInt)
    , m_drawType(drawType)
{
}

int Polyline::LineNum() const
{
    if (m_drawType == DrawType::Lines) {
        if (m_indexs.size() != 0) {
            return m_indexs.size() / 2;
        } else {
            return m_points.size() / 2;
        }
    } else  if (m_drawType == DrawType::LineLoop) {

    } else  if (m_drawType == DrawType::LineStrip) {

    }

    return 0;
}

void Polyline::AddLoop(const Polyline& point)
{
    STLUtil::Insert(m_points, point.m_points);
    m_points = CreateUnique();
    m_drawType = DrawType::LineLoop;
}


void Polyline::AddLoop(Polyline&& point)
{
    STLUtil::Insert(m_points, std::move(point.m_points));
    m_points = CreateUnique();
    m_drawType = DrawType::LineLoop;
}


void Polyline::AddCircle(Polyline&& circle)
{
    STLUtil::Insert(m_points, std::move(circle.m_points));
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
Vector<Vector3> Polyline::CreateTriangleLine() const
{
    auto triangles = CreateTriangleArray();
    Vector<Vector3> lines;
    for (int i = 0; i < triangles.size(); i+=3) {
        lines.push_back(triangles[i]);
        lines.push_back(triangles[i + 1]);

        lines.push_back(triangles[i + 1]);
        lines.push_back(triangles[i + 2]);

        lines.push_back(triangles[i + 2]);
        lines.push_back(triangles[i]);
    }

    return lines;
}
Vector<Vector3> Polyline::CreateTriangleArray() const
{
    DelaunayGenerator delaunay;
    delaunay.SetTarget(&m_points);
    return delaunay.Execute2D_CGAL();
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
    if (target.m_points.size() == 0) { return Vector<Vector3>(); }
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
    if (m_drawType == DrawType::LineLoop) {
        Vector<Vector3> points;
        for (size_t i = 0; i < m_points.size() - 1; i++) {
            points.push_back(m_points[i]);
            points.push_back(m_points[i + 1]);
        }

        points.push_back(m_points[m_points.size() - 1]);
        points.push_back(m_points[0]);
        return points;
    } else if(m_drawType == DrawType::Lines) {
        if (m_indexs.size() == 0) { return m_points; }
        Vector<Vector3> points;
        for (size_t i = 0; i < m_indexs.size(); i++) { points.push_back(m_points[m_indexs[i]]); }
        return points;
    } else if (m_drawType == DrawType::LineStrip) {
        Vector<Vector3> points;
        for (size_t i = 0; i < m_points.size() - 1; i++) {
            points.push_back(m_points[i]);
            points.push_back(m_points[i + 1]);
        }

        return points;
    }

    return Vector<Vector3>();

}
Vector3 Polyline::GetNormal() const
{
    if (m_points.size() < 3) return Vector3();
    // 平面の法線
    return glm::normalize(glm::cross(m_points[1] - m_points[0], m_points[2] - m_points[0]));

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

Polyline Polyline::CreateSmooth() const
{
    if (!(m_drawType == DrawType::LineLoop || m_drawType == DrawType::LineStrip)) { assert(0); return Polyline(); }
   auto lines = m_points;
   if (lines.size() == 0) { return Polyline(); }
   Vector<Vector3> polyline;
   polyline.push_back(lines.front());
   for (size_t i = 1; i < lines.size() - 1; i++) {
       auto begin = lines[i - 1];
       auto center = lines[i];
       auto end = lines[i + 1];

       auto line1 = center - begin;
       auto line2 = end - center;

       float len1 = glm::length(line1);
       float len2 = glm::length(line2);
	   if (len1 < MathHelper::EPS || len2 < MathHelper::EPS) { continue; }
       auto inner = glm::dot(line1/ len1, line2 / len2);
       if (inner < MathHelper::THR_RAD5) {
           polyline.push_back(lines[i]);
       }
   }
   
   return Polyline(std::move(polyline));
}


BDB Polyline::CreateBDB(const Polyline& polyline)
{
    BDB bdb;
    for (size_t i = 0; i < polyline.Get().size(); i++) {
        bdb.Add(polyline.Get()[i]);
    }

    return bdb;
}

Polyline Polyline::ToUV(const Polyline& polyline)
{
    if (!polyline.m_pUVConverter) { return polyline; }
    Vector<Vector3> uv;
    for (size_t i = 0; i < polyline.Get().size(); i++) {
        auto value = polyline.m_pUVConverter->toUV(polyline.Get()[i]);
        uv.push_back(Vector3(value.x, value.y, 0));
    }

    auto uvLine = Polyline(std::move(uv));
    uvLine.m_pUVConverter = polyline.m_pUVConverter;
    return uvLine;
}

Polyline Polyline::ToXYZ(const Polyline& polyline)
{
    if (!polyline.m_pUVConverter) { return polyline; }
    Vector<Vector3> xyz;
    for (size_t i = 0; i < polyline.Get().size(); i++) {
        xyz.push_back(polyline.m_pUVConverter->toXYZ(polyline.Get()[i]));
    }

    auto xyzLine = Polyline(std::move(xyz));
    xyzLine.m_pUVConverter = polyline.m_pUVConverter;
    return xyzLine;
}


void PolylineList::Add(PolylineList&& poly)
{
    for (int i = 0; i < poly.m_polylines.size(); i++) {
        m_polylines.push_back(std::move(poly.m_polylines[i]));
    }
}

Polyline PolylineList::Merge() const
{
    Polyline line;
    for (auto& polyline : m_polylines) {
        line.AddLoop(polyline);
    }

    return line;
}



}
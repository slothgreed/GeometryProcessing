#include "Polyline.h"
#include "KIMath.h"
#include "DelaunayGenerator.h"
#include "Utility.h"
#include "Mesh.h"
namespace KI
{

Polyline::Polyline(Vector<Vector3>&& points)
	: m_points(std::move(points)) 
    , m_drawType(DrawType::LineStrip)
{
}

Polyline::Polyline(Vector<Vector3>&& points, DrawType drawType)
    : m_points(std::move(points))
    , m_drawType(drawType)
{
}

Polyline::Polyline(Vector<Vector3>&& points, Vector<UInt>&& uInt, DrawType drawType)
    : m_points(std::move(points))
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
    } else  if (m_drawType == DrawType::LineStrip) {
        if (m_indexs.size() != 0) {
            return m_indexs.size() - 1;
        } else {
            return m_points.size() - 1;
        }
    } else  if (m_drawType == DrawType::LineLoop) {
        if (m_indexs.size() != 0) {
            return m_indexs.size();
        } else {
            return m_points.size();
        }
    }

    return 0;
}

void Polyline::Add(const Polyline& polyline)
{
    ConvertLines();
    auto p = polyline;
    p.ConvertLines();
    STLUtil::Insert(m_points, p.m_points);
}

void Polyline::Add(Polyline&& polyline)
{
    ConvertLines();
    auto p = polyline;
    p.ConvertLines();
    STLUtil::Insert(m_points, std::move(p.m_points));
}

void Polyline::Add(const Vector3& point)
{
    m_points.push_back(point);
}

void Polyline::ConvertLines()
{
    if (m_points.empty()) { m_drawType = DrawType::Lines; }
    if (m_drawType == DrawType::Lines) {
        if (m_indexs.empty()) { return; }
        Vector<Vector3> points(LineNum() * 2);
        for (size_t i = 0; i < m_indexs.size(); i++) {
            points[i] = m_points[m_indexs[i]];
        }
        m_points = std::move(points);
        m_indexs.clear();
    } else if (m_drawType == DrawType::LineStrip) {
        Vector<Vector3> points(LineNum() * 2);
        if (m_indexs.empty()) {
            for (size_t i = 0; i < m_points.size() - 1; i++) {
                points[2 * i] = m_points[i];
                points[2 * i + 1] = m_points[i + 1];
            }
        } else {
            for (size_t i = 0; i < m_indexs.size() - 1; i++) {
                points[2 * i] = m_points[m_indexs[i]];
                points[2 * i + 1] = m_points[m_indexs[i + 1]];
            }
            m_indexs.clear();
        }
        m_points = std::move(points);
        m_drawType = DrawType::Lines;
    } else if(m_drawType == DrawType::LineLoop){
        Vector<Vector3> points(LineNum() * 2);
        if (m_indexs.empty()) {
            for (size_t i = 0; i < m_points.size() - 1; i++) {
                points[2 * i] = m_points[i];
                points[2 * i + 1] = m_points[i + 1];
            }
            points[points.size() - 2] = m_points.back();
            points[points.size() - 1] = m_points.front();
        } else {
            for (size_t i = 0; i < m_indexs.size() - 1; i++) {
                points[2 * i] = m_points[m_indexs[i]];
                points[2 * i + 1] = m_points[m_indexs[i + 1]];
            }
            points[points.size() - 2] = m_points[m_indexs.back()];
            points[points.size() - 1] = m_points[m_indexs.front()];
            m_indexs.clear();
        }
        m_points = std::move(points);
        m_drawType = DrawType::Lines;
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

void Polyline::Reverse()
{
    if (m_indexs.size()) {
        std::reverse(m_indexs.begin(), m_indexs.end());
    } else {
        std::reverse(m_points.begin(), m_points.end());
    }
}

Mesh Polyline::CreateMesh() const
{
    DelaunayGenerator delaunay;
    delaunay.SetTarget(&m_points);
    return Mesh(delaunay.Execute2D_CGAL(), Mesh::DrawType::Triangles);
}
Mesh Polyline::CreateMesh(const Polyline& target, const Polyline& inner, bool orient)
{
    if (target.m_points.size() == 0) { return Mesh(); }
    Mesh mesh;
    auto normal = target.GetNormal();
    /*
    if (MathHelper::IsZ(normal)) {
        DelaunayGenerator delaunay;
        delaunay.SetTarget(&target.GetPoints());

        if (inner.GetPoints().size() != 0) {
            auto targetNormal = target.GetNormal();
            auto innerNormal = inner.GetNormal();
			if (!MathHelper::IsSame(target.GetNormal(), -inner.GetNormal())) { return Mesh(); }
            delaunay.AddInner(&inner.GetPoints());
        }
        mesh = Mesh(delaunay.Execute2D_CGAL(), Mesh::DrawType::Triangles);
    } else 
    */
    {
        DelaunayGenerator delaunay;
        auto info = MathHelper::CreateProjectInfo(target.GetPoints());
        auto zPosition = MathHelper::Project(target.GetPoints(), info);
        delaunay.SetTarget(&zPosition);
        Vector<Vector3> zInnerPosition;

        if (inner.GetPoints().size() != 0) {
			if (!MathHelper::IsSame(target.GetNormal(), -inner.GetNormal())) { return Mesh(); }
            zInnerPosition = MathHelper::Project(inner.GetPoints(), info);
            delaunay.AddInner(&zInnerPosition);
        }
        auto result = delaunay.Execute2D_CGAL();
        mesh = Mesh(MathHelper::UnProject(result, info), Mesh::DrawType::Triangles);
    }

    return mesh;
}

Vector3 Polyline::GetNormal() const
{
    if (m_points.size() < 3) return Vector3();
    return MathHelper::CalcNormal(m_points);
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
    for (size_t i = 0; i < polyline.GetPoints().size(); i++) {
        bdb.Add(polyline.GetPoints()[i]);
    }

    return bdb;
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
        line.Add(polyline);
    }

    return line;
}



}
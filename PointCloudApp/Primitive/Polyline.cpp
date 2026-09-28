#include "Primitive/Polyline.h"
#include "Utility/KIMath.h"
#include "Algorithm/DelaunayGenerator.h"
#include "Utility/Utility.h"
#include "Primitive/Mesh.h"
namespace KI
{


Vector<Vector3> IUVConverter::ConnectUV(const Vector<Vector3>& uv)
{
	if (uv.empty()) { return Vector<Vector3>(); }
	if (uv.size() % 2 != 0) { Assert::Failed(); return Vector<Vector3>(); }

	Vector<Vector3> points;
	points.reserve(uv.size() * 2);
	Vector3 begin = uv.front();
	for (size_t i = 0; i < uv.size(); i += 2) {
		const auto& end = uv[i + 1];
		if (!MathHelper::IsSame(begin, end)) {
			points.push_back(begin);
			points.push_back(end);
		}

		const auto& next = uv[(i + 2) % uv.size()];
		if (!MathHelper::IsSame(end, next)) {
			points.push_back(end);
			points.push_back(next);
			begin = next;
		} else {
			// Snap adjacent endpoints so CGAL sees an exactly connected boundary.
			begin = end;
		}
	}
	if (!points.empty() && MathHelper::IsSame(points.back(), points.front())) {
		points.back() = points.front();
	}
	return points;
}
Vector<Vector3> IUVConverter::ConvertUV(const Vector<Vector3>& target) const
{
	Vector<Vector3> uvs(target.size());
	for (size_t i = 0; i < uvs.size(); i++) {
		auto uv = toUV(target[i]);
		uvs[i] = Vector3(uv.x, uv.y, 0.0);
	}

	return uvs;
}
Vector<Vector3> IUVConverter::ConvertXYZ(const Vector<Vector3>& target) const
{
	Vector<Vector3> uvs(target.size());
	for (size_t i = 0; i < uvs.size(); i++) {
		auto uv = toXYZ(target[i]);
		uvs[i] = Vector3(uv.x, uv.y, uv.z);
	}

	return uvs;
}
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
    if (polyline.GetPoints().empty()) { return; }
    ConvertLines();
    auto p = polyline;
    p.ConvertLines();
    if (!m_points.empty()) {
        if (MathHelper::IsSame(m_points.back(), p.m_points.front())) {
            p.m_points.front() = m_points.back();
        }

        if (MathHelper::IsSame(p.m_points.back(), m_points.front())) {
            m_points.front() = p.m_points.back();
        }
    }

    STLUtil::Insert(m_points, p.m_points);
}

void Polyline::Add(Polyline&& polyline)
{
    if (polyline.GetPoints().empty()) { return; }
    ConvertLines();
    auto p = std::move(polyline);
    p.ConvertLines();
    if (!m_points.empty()) {
        if (MathHelper::IsSame(m_points.back(), p.m_points.front())) {
            p.m_points.front() = m_points.back();
        }

        if (MathHelper::IsSame(p.m_points.back(), m_points.front())) {
            m_points.front() = p.m_points.back();
        }
    }
    STLUtil::Insert(m_points, std::move(p.m_points));
}

void Polyline::Add(const Vector3& point)
{
    m_points.push_back(point);
}

std::pair<Vector3, Vector3> Polyline::GetLine(int index) const
{
    if (m_indexs.empty()) {
        if (m_drawType == Polyline::DrawType::Lines) {
            return  std::pair<Vector3, Vector3>(m_points[2 * index], m_points[2 * index + 1]);
        } else {
            Assert::Failed();
        }
    } else {
        if (m_drawType == Polyline::DrawType::Lines) {
            return  std::pair<Vector3, Vector3>(m_points[m_indexs[2 * index]], m_points[m_indexs[2 * index + 1]]);
        } else {
            Assert::Failed();
        }
    }
    return std::pair<Vector3,Vector3>();
}

Polyline& Polyline::ConvertLines()
{
    if (m_points.empty()) { m_drawType = DrawType::Lines; }
    if (m_drawType == DrawType::Lines) {
        if (m_indexs.empty()) { return *this; }
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
    } else if (m_drawType == DrawType::LineLoop) {
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

    return *this;
}
Vector3 Polyline::GetCenter() const
{
    Vector3 center = Vector3(0);
    for (const auto& p : m_points) {
        center += p;
    }

    return center /= m_points.size();
}

Polyline& Polyline::Reverse()
{
    if (m_indexs.size()) {
        std::reverse(m_indexs.begin(), m_indexs.end());
    } else {
        std::reverse(m_points.begin(), m_points.end());
    }

    return *this;
}

Mesh Polyline::CreateMesh() const
{
    DelaunayGenerator delaunay;
    delaunay.SetTarget(&m_points);
    return Mesh(delaunay.Execute2D_CGAL(), Mesh::DrawType::Triangles);
}

Mesh Polyline::CreateMesh(const Polyline& target, const Polyline& inner, const IUVConverter& converter, bool hasBusLine, const Vector3& axis)
{
	if (target.m_points.size() == 0) { return Mesh(); }
	DelaunayGenerator delaunay;
	auto uvTarget = converter.ConvertUV(target.GetPoints());
	DebugPrintf::Vec3Array("Polyline::CreateMesh uvTarget", uvTarget);
	if (!hasBusLine) {
		uvTarget = converter.ConnectUV(uvTarget);
	}
	delaunay.SetTarget(&uvTarget);
	Vector<Vector3> uvInner;
	if (inner.GetPoints().size() != 0) {
		uvInner = converter.ConvertUV(inner.GetPoints());
		delaunay.AddInner(&uvInner);
	}
	auto result = delaunay.Execute2D_CGAL();
	if (result.empty()) { return Mesh(); }

	return Mesh(std::move(converter.ConvertXYZ(result)), Mesh::DrawType::Triangles);

}
Mesh Polyline::CreateMesh(const Polyline& target, const Polyline& inner, const IUVConverter& converter, const Vector3& axis)
{
	if (target.m_points.size() == 0) { return Mesh(); }
	DelaunayGenerator delaunay;
	auto uvTarget = converter.ConvertUV(target.GetPoints());
	delaunay.SetTarget(&uvTarget);
	Vector<Vector3> uvInner;
	if (inner.GetPoints().size() != 0) {
		uvInner = converter.ConvertUV(inner.GetPoints());
		delaunay.AddInner(&uvInner);
	}
	auto result = delaunay.Execute2D_CGAL();
	if (result.empty()) { return Mesh(); }

	return Mesh(std::move(converter.ConvertXYZ(result)), Mesh::DrawType::Triangles);
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
    if (!(m_drawType == DrawType::LineLoop || m_drawType == DrawType::LineStrip)) { Assert::Failed(); return Polyline(); }
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

Vector<Vector3> Polyline::CreateParametricColor() const
{
    if (m_points.empty()) {
        return Vector<Vector3>();
    }
    Vector<Vector3> colors;
    if (m_drawType == DrawType::Lines) {
        if (m_indexs.empty()) {
            float sumLen = 0.0f;
            for (size_t i = 0; i < m_points.size(); i+=2) {
               sumLen += glm::length2(m_points[i] - m_points[i + 1]);
            }
            float offset = 0.0f;
            for (size_t i = 0; i < m_points.size(); i += 2) {
                auto len = glm::length2(m_points[i] - m_points[i + 1]);
                colors.push_back(ColorUtility::CreatePseudo(offset, sumLen));
                colors.push_back(ColorUtility::CreatePseudo(offset + len, sumLen));
                offset += len;
            }
            return colors;
        }
    }

    Assert::Failed();
    return colors;
}

void PolylineList::Add(PolylineList&& poly)
{
    for (int i = 0; i < poly.m_polylines.size(); i++) {
        m_polylines.push_back(std::move(poly.m_polylines[i]));
    }
}

BDB PolylineList::CreateBDB() const
{
    BDB bdb;
    for (int i = 0; i < m_polylines.size(); i++) {
        bdb.Add(BDB(m_polylines[i].second.GetPoints()));
    }
    return bdb;
}
Polyline PolylineList::CreateMerge() const
{
    Polyline line;
    for (auto& polyline : m_polylines) {
        line.Add(polyline.second);
    }

    return line;
}

}

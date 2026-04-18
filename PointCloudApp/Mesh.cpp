#include "Mesh.h"

namespace KI
{
int Mesh::TriangleNum() const
{
	if (m_drawType == DrawType::Triangles) {
		if (m_indexs.size() != 0) {
			return m_indexs.size() / 3;
		} else {
			return m_points.size() / 3;
		}
	} else if (
		m_drawType == DrawType::TriangleFan ||
		m_drawType == DrawType::TriangleStrip) {
		if (m_indexs.size() != 0) {
			return m_indexs.size() - 2;
		} else {
			return m_points.size() - 2;
		}
	}

	return 0;
}

Mesh::Triangle Mesh::GetTriangle(int index) const
{
	Triangle tri;
	if (m_drawType == DrawType::Triangles) {
		if (m_indexs.empty()) {
			tri.pos0 = m_points[3 * index + 0];
			tri.pos1 = m_points[3 * index + 1];
			tri.pos2 = m_points[3 * index + 2];
			return tri;
		} else {
			tri.pos0 = m_points[m_indexs[3 * index + 0]];
			tri.pos1 = m_points[m_indexs[3 * index + 1]];
			tri.pos2 = m_points[m_indexs[3 * index + 2]];
			return tri;
		}
	} else {
		assert(0);
		return tri;
	}
}

Mesh& Mesh::ConvertTriangles()
{
	if (m_drawType == DrawType::Triangles) {
		if (m_indexs.empty()) { return *this; }
		Vector<Vector3> points(m_indexs.size());
		for (size_t i = 0; i < m_indexs.size(); i++) {
			points[i] = m_points[m_indexs[i]];
		}
		m_points = std::move(points);
		m_indexs.clear();
	} else {
		assert(0);
	}

	return *this;
}

Mesh& Mesh::Reverse()
{
	if (m_drawType == DrawType::Triangles) {
		if (m_indexs.empty()) {
			Vector<Vector3> points(m_points.size());
			for (size_t i = 0; i < m_points.size(); i += 3) {
				points[i] = m_points[i];
				points[i + 1] = m_points[i + 2];
				points[i + 2] = m_points[i + 1];
			}
			m_points = std::move(points);
		} else {
			Vector<UInt> indexs(m_indexs.size());
			for (size_t i = 0; i < m_indexs.size(); i += 3) {
				indexs[i] = m_indexs[i];
				indexs[i + 1] = m_indexs[i + 2];
				indexs[i + 2] = m_indexs[i + 1];
			}
			m_indexs = std::move(indexs);
		}
	} else {
		assert(0);
	}

	return *this;
}
}
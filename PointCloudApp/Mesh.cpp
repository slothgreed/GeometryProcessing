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

BDB Mesh::CreateBDB(const Mesh& mesh)
{
	BDB bdb;
	for (size_t i = 0; i < mesh.GetPoints().size(); i++) {
		bdb.Add(mesh.GetPoints()[i]);
	}

	return bdb;
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
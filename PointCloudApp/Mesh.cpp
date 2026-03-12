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
}
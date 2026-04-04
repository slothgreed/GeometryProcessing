#ifndef MESH_H
#define MESH_H
#include "BDB.h"
namespace KI
{

class Mesh
{
public:
	enum class DrawType
	{
		Triangles = GL_TRIANGLES,
		TriangleStrip = GL_TRIANGLE_STRIP,
		TriangleFan = GL_TRIANGLE_FAN
	};

	Mesh() = default;
	Mesh(Vector<Vector3>&& points, Vector<UInt>&& index, Mesh::DrawType drawType)
		: m_points(std::move(points))
		, m_indexs(std::move(index))
		, m_drawType(drawType)
	{
	}

	Mesh(Vector<Vector3>&& points, Mesh::DrawType drawType)
		: m_points(std::move(points))
		, m_drawType(drawType)
	{
	}
	~Mesh() {};

	int TriangleNum() const;
	const Vector<Vector3>& GetPoints() const { return m_points; }
	const Vector<UInt>& GetIndexs() const { return m_indexs; }
	GLuint GetDrawType() const { return (GLuint)m_drawType; }
	Mesh& ConvertTriangles();
	Mesh& Reverse();
private:
	Vector<Vector3> m_points;
	Vector<UInt> m_indexs;
	DrawType m_drawType = DrawType::Triangles;
};

}


#endif MESH_H
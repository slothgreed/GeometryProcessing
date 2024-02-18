#ifndef IPRIMITIVE_H
#define IPRIMITIVE_H
#include "BDB.h"
namespace KI
{
struct Vertex
{
	Vector3 m_position;
	Vector3 m_normal;
};

class Primitive
{
public:
	Primitive();
	virtual ~Primitive() {};

	enum StoreType
	{
		Array,
		Interleave
	};
public:
	const BDB& GetBDB();
	GLuint GetDrawType() const;
	void CalcNormal(); // 適当コード
	bool IsInterleave() const { return m_storeType == StoreType::Interleave; }
	StoreType GetStoreType() const { return m_storeType; }

	const Vector<Vector3>& Position() const { return m_position; };
	const Vector<Vector3>& Normal() const { return m_normal; };
	const Vector<Vector3>& Color() const { return m_color; };
	const Vector<glm::vec2>& Texcoord() const { return m_texcoord; };
	const Vector<Vertex>& GetVertex() const { return m_vertex; };
	const Vector<unsigned int>& Index() const { return m_index; };

	void SetPosition(Vector<Vector3>&& value) { m_position = std::move(value); }
	void SetNormal(Vector<Vector3>&& value) { m_normal = std::move(value); }
	void SetColor(Vector<Vector3>&& value) { m_color = std::move(value); }
	void SetTexcoord(Vector<glm::vec2>&& value) { m_texcoord = std::move(value); }

	void SetIndex(Vector<unsigned int>&& value) { m_index = std::move(value); }
	void Multi(const Matrix4x4& matrix);
	void Convert(StoreType type);
	Shared<Primitive> Clone();
	int GetTriangleNum();
	void SetType(GLuint primitive) { m_primitiveType = primitive; }
	GLuint GetType() const { return m_primitiveType; }
	bool NeedUpdate() const { return m_update; }
	void ClearUpdate() { m_update = false; }
	void Update() { m_update = true; }
protected:
	bool m_update;
	StoreType m_storeType;
	Vector<Vector3> m_color;
	Vector<Vector3> m_position;
	Vector<Vector3> m_normal;
	Vector<glm::vec2> m_texcoord;
	Vector<Vertex> m_vertex;
	GLuint m_primitiveType;
	Vector<unsigned int> m_index;
	BDB m_bdb;
	
};
}
#endif

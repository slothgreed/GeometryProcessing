#ifndef IPRIMITIVE_H
#define IPRIMITIVE_H
#include "BDB.h"
struct Vertex
{
	glm::vec3 m_position;
	glm::vec3 m_normal;
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
	GLuint GetDrawType();
	void CalcNormal(); // 適当コード
	bool IsInterleave() { return m_storeType == StoreType::Interleave; }
	StoreType GetStoreType() { return m_storeType; }
	void SetPosition(Vector<glm::vec3>&& value) { m_position = std::move(value); }
	Vector<glm::vec3>& Position() { return m_position; };
	void SetNormal(Vector<glm::vec3>&& value) { m_normal = std::move(value); }
	Vector<glm::vec3>& Normal() { return m_normal; };
	void SetColor(Vector<glm::vec3>&& value) { m_color = std::move(value); }
	Vector<glm::vec3>& Color() { return m_color; };
	void SetTexcoord(Vector<glm::vec2>&& value) { m_texcoord = std::move(value); }
	Vector<glm::vec2>& Texcoord() { return m_texcoord; };

	Vector<Vertex>& GetVertex() { return m_vertex; };
	void SetIndex(Vector<unsigned int>&& value) { m_index = std::move(value); }
	Vector<unsigned int>& Index() { return m_index; };
	void Multi(const mat4x4& matrix);
	void Convert(StoreType type);
	Shared<Primitive> Clone();
	int GetTriangleNum();
	void SetType(GLuint primitive) { m_primitiveType = primitive; }
	GLuint GetType() { return m_primitiveType; }
	bool NeedUpdate() { return m_update; }
	void ClearUpdate() { m_update = false; }
	void Update() { m_update = true; }
protected:
	bool m_update;
	StoreType m_storeType;
	Vector<glm::vec3> m_color;
	Vector<glm::vec3> m_position;
	Vector<glm::vec3> m_normal;
	Vector<glm::vec2> m_texcoord;
	Vector<Vertex> m_vertex;
	GLuint m_primitiveType;
	Vector<unsigned int> m_index;
	BDB m_bdb;
	
};

#endif

#include "Primitive.h"
#include "BDB.h"
namespace KI
{
Primitive::Primitive()
{
	m_update = true;
	m_primitiveType = GL_NONE;
	m_storeType = StoreType::Array;
}

GLuint Primitive::GetDrawType() const
{
	if (m_primitiveType == GL_NONE)
	{
		assert(0);
		return GL_POINTS;
	}
	return m_primitiveType;
}

void Primitive::Multi(const Matrix4x4& matrix)
{
	if (m_storeType == StoreType::Array)
	{
		for (int i = 0; i < m_position.size(); i++)
		{
			m_position[i] = (matrix * vec4(m_position[i], 1.0));
		}
	}
	else
	{
		for (int i = 0; i < m_vertex.size(); i++)
		{
			m_vertex[i].m_position = (matrix * vec4(m_vertex[i].m_position, 1.0));
		}
	}
}

void Primitive::CalcNormal()
{
	m_normal.resize(m_position.size());
	for (int i = 0; i < m_position.size(); i ++)
	{
		if (m_position[i] == Vector3(0))
		{
			m_normal[i] = Vector3(0);
		}
		else
		{
			m_normal[i] = glm::normalize(m_position[i]);
		}
	}
}

void Primitive::Convert(Primitive::StoreType type)
{
	if (m_storeType == type)
	{
		return;
	}

	if (type == StoreType::Interleave)
	{
		m_vertex.resize(m_position.size());
		for (int i = 0; i < m_vertex.size(); i++)
		{
			m_vertex[i].m_position = m_position[i];
			m_vertex[i].m_normal = m_normal[i];
		}

		m_position.clear();
		m_normal.clear();
	}
	else 
	{
		m_position.resize(m_vertex.size());
		m_normal.resize(m_vertex.size());

		for (int i = 0; i < m_vertex.size(); i++)
		{
			m_position[i] = m_vertex[i].m_position;
			m_normal[i] = m_vertex[i].m_normal;
		}

		m_vertex.clear();
	}

	m_storeType = type;
	
}

Shared<Primitive> Primitive::Clone()
{
	auto instance = std::make_shared<Primitive>();
	instance->m_primitiveType = m_primitiveType;
	instance->m_storeType = m_storeType;

	instance->m_position = m_position;
	instance->m_normal = m_normal;
	instance->m_color = m_color;
	instance->m_texcoord = m_texcoord;
	instance->m_vertex = m_vertex;
	
	instance->m_index = m_index;
	return instance;
}

const BDB& Primitive::GetBDB()
{
	if (m_bdb.IsActive()) {
		return m_bdb;
	}

	for (const auto& p : m_position)
	{
		m_bdb.Add(p);
	}

	return m_bdb;
}
int Primitive::GetTriangleNum()
{
	if (m_index.size() != 0)
	{
		return m_index.size() / 3;
	}
	else
	{
		return m_position.size() / 3;
	}
}


void Primitive::AddTriangle(Vector<unsigned int>& index, int vertex0, int vertex1, int vertex2, int vertex3)
{
	index.push_back(vertex0); index.push_back(vertex1); index.push_back(vertex2);
	index.push_back(vertex0); index.push_back(vertex2); index.push_back(vertex3);
}
}
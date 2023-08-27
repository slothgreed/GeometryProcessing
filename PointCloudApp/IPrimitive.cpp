#include "IPrimitive.h"
#include "BDB.h"
IPrimitive::IPrimitive()
{
	m_update = true;
	m_drawType = GL_NONE;
	m_storeType = StoreType::Array;
}

GLuint IPrimitive::GetDrawType()
{
	if (m_drawType == GL_NONE)
	{
		assert(0);
		return GL_POINTS;
	}
	return m_drawType;
}

void IPrimitive::Multi(const mat4x4& matrix)
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

void IPrimitive::CalcNormal()
{
	m_normal.resize(m_position.size());
	for (int i = 0; i < m_position.size(); i ++)
	{
		if (m_position[i] == vec3(0))
		{
			m_normal[i] = vec3(0);
		}
		else
		{
			m_normal[i] = glm::normalize(m_position[i]);
		}
	}
}

void IPrimitive::Convert(IPrimitive::StoreType type)
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

std::shared_ptr<IPrimitive> IPrimitive::Clone()
{
	auto instance = make_shared<IPrimitive>();
	instance->m_drawType = m_drawType;
	instance->m_storeType = m_storeType;

	instance->m_position = m_position;
	instance->m_normal = m_normal;
	instance->m_vertex = m_vertex;
	
	instance->m_index = m_index;
	return instance;
}

BDB IPrimitive::CreateBDB() const
{
	BDB bdb;
	for (const auto& p : m_position)
	{
		bdb.Add(p);
	}

	return bdb;
}
int IPrimitive::GetTriangleNum()
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
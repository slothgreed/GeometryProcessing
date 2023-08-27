#include "GLBuffer.h"
GLBuffer::GLBuffer(GLenum type)
	:m_type(type),
	m_id(0),
	m_size(0)
{
}

GLBuffer::~GLBuffer()
{
	Delete();
}

void GLBuffer::Create(int size, int sizeofData)
{
	Delete();
	glCreateBuffers(1, &m_id);
	glNamedBufferData(m_id, size * sizeofData, nullptr, GL_STATIC_DRAW);
	m_size = size * sizeofData;
}

void GLBuffer::Create(const std::vector<glm::vec3>& value)
{
	Delete();
	glCreateBuffers(1, &m_id);
	glNamedBufferData(m_id, value.size() * sizeof(glm::vec3), value.data(), GL_STATIC_DRAW);
	m_size = value.size();
	OUTPUT_GLERROR;
}

void GLBuffer::Create(const std::vector<int>& value)
{
	Delete();
	glCreateBuffers(1, &m_id);
	glNamedBufferData(m_id , value.size() * sizeof(int), value.data(), GL_STATIC_DRAW);
	m_size = value.size();
	OUTPUT_GLERROR;
}

void GLBuffer::Create(const std::vector<mat4x4>& value)
{
	Delete();
	glCreateBuffers(1, &m_id);
	glNamedBufferData(m_id, value.size() * sizeof(mat4x4), value.data(), GL_STATIC_DRAW);
	OUTPUT_GLERROR;
}

void GLBuffer::Delete() 
{
	if (m_id != 0)
	{
		glDeleteBuffers(1, &m_id);
		OUTPUT_GLERROR;
		m_id = 0;
	}
}

void GLBuffer::BufferSubData(int offset, const std::vector<int>& value)
{
	assert(m_id != 0);
	glNamedBufferSubData(m_id, offset * sizeof(int), value.size() * sizeof(int), value.data());
	OUTPUT_GLERROR;
}

void GLBuffer::BufferSubData(int offset, const std::vector<glm::vec3>& value)
{
	assert(m_id != 0);
	glNamedBufferSubData(m_id, offset * sizeof(glm::vec3), value.size() * sizeof(glm::vec3), value.data());
	OUTPUT_GLERROR;
}


int GLBuffer::Size()
{
	return m_size;
}
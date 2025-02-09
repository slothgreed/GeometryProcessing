#include "GLBuffer.h"
namespace KI
{
GLBuffer::GLBuffer()
	: m_handle(0)
	, m_memorySize(0)
	, m_sizeOfData(0)
	, m_num(0)
	, m_dataType(DATA_UNKNOWN)
{
}

GLBuffer::~GLBuffer()
{
	Delete();
}

void GLBuffer::Create(DATA_TYPE dataType, int size, int sizeofData, const void* data)
{
	Delete();
	glCreateBuffers(1, &m_handle);
	OUTPUT_GLERROR;
	glNamedBufferData(m_handle, size * sizeofData, data, GL_STATIC_DRAW);
	OUTPUT_GLERROR;
	m_dataType = dataType;
	m_num = size;
	m_memorySize = size * sizeofData;
	m_sizeOfData = sizeofData;
}

void GLBuffer::Create(int size, int sizeofData)
{
	Create(DATA_UNKNOWN, size, sizeofData, NULL);
}

void GLBuffer::Create(const Vector<glm::vec2>& value)
{
	Create(DATA_FLOAT, value.size(), sizeof(glm::vec2), value.data());
}

void GLBuffer::Create(const Vector<Vector3>& value)
{
	Create(DATA_FLOAT, value.size(), sizeof(Vector3), value.data());
}

void GLBuffer::Create(const Vector<unsigned short>& value)
{
	Create(DATA_USHORT, value.size(), sizeof(unsigned short), value.data());
}

void GLBuffer::Create(const Vector<glm::vec4>& value)
{
	Create(DATA_FLOAT, value.size(), sizeof(glm::vec4), value.data());
}


void GLBuffer::Create(const Vector<unsigned int>& value)
{
	Create(DATA_UINT, value.size(), sizeof(unsigned int), value.data());
}

void GLBuffer::Create(const Vector<float>& value)
{
	Create(DATA_FLOAT, value.size(), sizeof(float), value.data());
}

void GLBuffer::Create(const Vector<Matrix4x4>& value)
{
	Create(DATA_FLOAT, value.size(), sizeof(Matrix4x4), value.data());
}

void GLBuffer::Delete()
{
	if (m_handle == 0) { return; }
	glDeleteBuffers(1, &m_handle);
	OUTPUT_GLERROR;
	m_handle = 0;
}

void GLBuffer::BufferSubData(int offset, const Vector<int>& value)
{
	assert(m_handle != 0);
	glNamedBufferSubData(m_handle, offset * sizeof(int), value.size() * sizeof(int), value.data());
	m_dataType = DATA_INT;
	OUTPUT_GLERROR;
}

void GLBuffer::BufferSubData(int offset, const Vector<Vector3>& value)
{
	assert(m_handle != 0);
	glNamedBufferSubData(m_handle, offset * sizeof(Vector3), value.size() * sizeof(Vector3), value.data());
	m_dataType = DATA_FLOAT;
	OUTPUT_GLERROR;
}

void GLBuffer::BufferSubData(int offset, int size, const void* data)
{
	assert(m_handle != 0);
	glNamedBufferSubData(m_handle, offset, size, data);
	OUTPUT_GLERROR;
}

void GLBuffer::BufferSubData(int offset, int size, int sizeofData, const void* data)
{
	assert(m_handle != 0);
	glNamedBufferSubData(m_handle, offset * sizeofData, size * sizeofData, data);
	OUTPUT_GLERROR;
}

void GLBuffer::GetBufferData(Vector<int>& value)
{
	assert(m_handle != 0);
	glGetNamedBufferSubData(m_handle, 0, value.size() * sizeof(int), value.data());
	OUTPUT_GLERROR;
}

void GLBuffer::GetBufferData(Vector<float>& value)
{
	assert(m_handle != 0);
	glGetNamedBufferSubData(m_handle, 0, value.size() * sizeof(float), value.data());
	OUTPUT_GLERROR;
}

void GLBuffer::GetBufferData(Vector<Vector3>& value)
{
	assert(m_handle != 0);
	glGetNamedBufferSubData(m_handle, 0, value.size() * sizeof(Vector3), value.data());
	OUTPUT_GLERROR;
}

void GLBuffer::GetBufferData(Vector<glm::vec4>& value)
{
	assert(m_handle != 0);
	glGetNamedBufferSubData(m_handle, 0, value.size() * sizeof(glm::vec4), value.data());
	OUTPUT_GLERROR;
}

void GLBuffer::GetBufferData(void* value, int memorySize)
{
	assert(m_handle != 0);
	glGetNamedBufferSubData(m_handle, 0, memorySize, value);
	OUTPUT_GLERROR;
}

void GLBuffer::SetData(unsigned int value)
{
	assert(m_handle != 0);
	glClearNamedBufferSubData(m_handle, GL_R32UI, 0, m_memorySize, GL_RED, GL_UNSIGNED_INT, &value);
	OUTPUT_GLERROR;
}
int GLBuffer::ComponentSize() const
{
	if (m_dataType == DATA_FLOAT ||
		m_dataType == DATA_INT ||
		m_dataType == DATA_UINT) {
		return m_sizeOfData / 4;
	} else {
		assert(0);
		return 0;
	}
}
}

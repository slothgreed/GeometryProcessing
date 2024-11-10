#ifndef GL_BUFFER_H
#define GL_BUFFER_H
#include "GLTypes.h"
namespace KI
{

enum class BUFFER_TYPE
{
	ARRAY,
	ELEMENT_ARRAY
};

class GLBuffer
{
public:
	GLBuffer();
	~GLBuffer();


	GLuint Handle() const { return m_handle; }
	bool Created() const { return m_handle != 0; }
	void Delete();
	void Create(const Vector<glm::vec2>& value);
	void Create(const Vector<Vector3>& value);
	void Create(const Vector<unsigned short>& value);
	void Create(const Vector<glm::vec4>& value);
	void Create(const Vector<unsigned int>& value);
	void Create(const Vector<Matrix4x4>& value);
	void Create(const Vector<float>& value);
	void Create(int size, int sizeofData);
	void Create(DATA_TYPE dataType, int size, int sizeofData, const void* data);

	template <typename T> void Create(const Vector<T>& value)
	{
		Create((int)value.size(), sizeof(T));
		BufferSubData(0, value);
	}
	void BufferSubData(int offset, const Vector<int>& value);
	void BufferSubData(int offset, int size, int sizeofData, const void* data);
	void BufferSubData(int offset, const Vector<Vector3>& value);
	void BufferSubData(int offset, int size, const void* data);
	template <typename T>
	void BufferSubData(int offset, const Vector<T>& value)
	{
		BufferSubData(DATA_UNKNOWN, (int)value.size(), sizeof(T), value.data());
	}
	void GetBufferData(Vector<int>& value);
	void GetBufferData(Vector<float>& value);
	void GetBufferData(Vector<Vector3>& value);
	void GetBufferData(Vector<glm::vec4>& value);
	void GetBufferData(void* value, int memorySize);

	GLuint DataType() const { return (GLuint)m_dataType; }
	int Num() const { return m_num; }
	int MemorySize() const { return m_num * m_sizeOfData; }
	int SizeOfData() const { return m_sizeOfData; }
	int ComponentSize() const;
private:
	int m_memorySize;
	int m_sizeOfData;
	int m_num;
	DATA_TYPE m_dataType;
	GLuint m_handle;
};

struct VertexFormat
{
	VertexFormat()
		: location(-1)
		, componentSize(1)
		, type(DATA_FLOAT)
		, normalized(false)
		, offset(0)
		, binding(0)
	{
	}

	VertexFormat(int _location, const GLBuffer* pBuffer)
		: location(_location)
		, binding(_location)
		, componentSize(pBuffer->ComponentSize())
		, type((DATA_TYPE)pBuffer->DataType())
		, normalized(false)
		, offset(0)
	{
	}
	String name;
	int location;
	int componentSize;
	DATA_TYPE type;
	bool normalized;
	int binding;
	unsigned int offset;
};

using VertexFormats = Vector<VertexFormat>;

}


#endif GL_BUFFER_H
#ifndef GL_BUFFER_H
#define GL_BUFFER_H
#include "GLTypes.h"
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
	void Create(const std::vector<glm::vec2>& value);
	void Create(const std::vector<glm::vec3>& value);
	void Create(const std::vector<glm::vec4>& value);
	void Create(const std::vector<unsigned int>& value);
	void Create(const std::vector<mat4x4>& value);
	void Create(const std::vector<float>& value);
	void Create(int size, int sizeofData);
	void Create(DATA_TYPE dataType, int size, int sizeofData, const void* data);
	void BufferSubData(int offset, const std::vector<int>& value);
	void BufferSubData(int offset, int size, int sizeofData, const void* data);
	void BufferSubData(int offset, const std::vector<glm::vec3>& value);
	void GetBufferData(std::vector<int>& value);
	void GetBufferData(std::vector<float>& value);
	void GetBufferData(std::vector<glm::vec3>& value);
	void GetBufferData(std::vector<glm::vec4>& value);

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


#endif GL_BUFFER_H
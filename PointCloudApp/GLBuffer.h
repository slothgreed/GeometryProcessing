#ifndef GL_BUFFER_H
#define GL_BUFFER_H
enum class BUFFER_TYPE
{
	ARRAY,
	ELEMENT_ARRAY
};

class GLBuffer
{
public:
	GLBuffer(GLenum type);
	~GLBuffer();


	bool Created() { return m_id != 0; }
	void Delete();
	void Create(const std::vector<glm::vec3>& value);
	void Create(const std::vector<int>& value);
	void Create(const std::vector<mat4x4>& value);

	void Create(int size, int sizeofData);
	void BufferSubData(int offset, const std::vector<int>& value);
	void BufferSubData(int offset, const std::vector<glm::vec3>& value);
	GLuint GetId() { return m_id; }
	int Size();
private:
	int m_size;
	GLenum m_type;
	GLuint m_id;
};


#endif GL_BUFFER_H
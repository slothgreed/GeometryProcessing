#ifndef FRAME_BUFFER_H
#define	FRAME_BUFFER_H
namespace KI
{

class FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer();

	void Build();
	void Bind();
	void UnBind();
	void Delete();
	GLuint Handle() { return m_handle; }
	bool Validate() const;
private:
	GLuint m_handle;
};
}

#endif FRAME_BUFFER_H
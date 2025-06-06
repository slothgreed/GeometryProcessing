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
	static void UnBind();
	void Delete();
	GLuint Handle() const { return m_handle; }
	bool Validate() const;
private:
	GLuint m_handle;
};
}

#endif FRAME_BUFFER_H
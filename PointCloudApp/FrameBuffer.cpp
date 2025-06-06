#include "FrameBuffer.h"
namespace KI
{
FrameBuffer::FrameBuffer()
	:m_handle(0)
{
}

FrameBuffer::~FrameBuffer()
{
	Delete();
}


void FrameBuffer::Build()
{
	glGenFramebuffers(1, &m_handle);
	OUTPUT_GLERROR;
}


void FrameBuffer::Bind()
{
	if (m_handle == 0) { return; }
	glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
	OUTPUT_GLERROR;
}



void FrameBuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	OUTPUT_GLERROR;
}


void FrameBuffer::Delete()
{
	if (m_handle == 0) { return; }
	glDeleteFramebuffers(1, &m_handle);
	OUTPUT_GLERROR;
}


bool FrameBuffer::Validate() const
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		assert(0);
		return false;
	}

	return true;
}

}

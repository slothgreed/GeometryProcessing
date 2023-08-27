#include "MatrixTexture.h"

MatrixTexture::MatrixTexture()
{
	glCreateTextures(GL_TEXTURE_BUFFER, 1, &m_Id);
}

MatrixTexture::~MatrixTexture()
{
	Delete();
}

void MatrixTexture::TextureBuffer(int matrixId)
{
	glTextureBuffer(m_Id, GL_RGBA32F, matrixId);
}

void MatrixTexture::Bind()
{
	glBindTexture(GL_TEXTURE_BUFFER, m_Id);
}
void MatrixTexture::Delete()
{
	if (m_Id != 0)
	{
		glDeleteTextures(1, &m_Id);
	}
}
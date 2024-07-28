#include "Texture.h"
#include "TextureLoader.h"
namespace KI
{

Texture::Texture()
{

}

Texture::~Texture()
{
}

void Texture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, m_handle);
	OUTPUT_GLERROR;
}
void Texture::Delete()
{
	if (m_handle == 0) return;
	glDeleteTextures(1, &m_handle);
	OUTPUT_GLERROR;
}

TextureBuffer::TextureBuffer()
{
	glCreateTextures(GL_TEXTURE_BUFFER, 1, &m_handle);
	OUTPUT_GLERROR;
}
TextureBuffer::~TextureBuffer()
{

}
void TextureBuffer::Bind(int bufferId)
{
	glTextureBuffer(m_handle, GL_RGBA32F, bufferId);
	OUTPUT_GLERROR;
}

Texture2D::Texture2D()
{
	glGenTextures(1, &m_handle);
}

void Texture2D::Clear(int value)
{
	glBindTexture(GL_TEXTURE_2D, m_handle);
	glClearTexImage(m_handle, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

}
void Texture2D::Build(int width, int height)
{
	Build(width, height, NULL);
}

void Texture2D::Resize(int width, int height)
{
	glBindTexture(GL_TEXTURE_2D, m_handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	m_size.x = width;
	m_size.y = height;
	OUTPUT_GLERROR;
}

void Texture2D::Build(int width, int height, unsigned char* data)
{
	glBindTexture(GL_TEXTURE_2D, m_handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	m_size.x = width;
	m_size.y = height;
	OUTPUT_GLERROR;

}

CubemapTexture::CubemapTexture()
{
	glGenTextures(1, &m_handle);
	OUTPUT_GLERROR;
}
void CubemapTexture::Build(const Vector<String>& path)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);

	for (size_t i = 0; i < path.size(); i++) {
		auto pixelData  = TextureLoader::LoadData(path[i], 0);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, pixelData->width, pixelData->height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData->data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}
}

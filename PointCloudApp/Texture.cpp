#include "Texture.h"
#include "TextureLoader.h"
namespace KI
{

Texture::Texture()
	:m_handle(0)
{

}

Texture::~Texture()
{
}

void Texture::Set(const Format& format)
{
	Bind();
	glTexImage2D(
		format.target, format.level,
		format.internalformat,
		format.width, format.height,
		format.border, format.format,
		format.type, nullptr);
	m_format = format;
	OUTPUT_GLERROR;
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
	glClearTexImage(m_handle, 0, m_format.format, m_format.type, &value);

}
void Texture2D::Build(int width, int height)
{
	Build(width, height, NULL);
}

void Texture2D::Resize(int width, int height)
{
	if (m_format.width == width && m_format.height == height) { return; }
	m_format.width = width;
	m_format.height = height;
	Set(m_format);
	OUTPUT_GLERROR;
}

void Texture2D::Build(int width, int height, unsigned char* data)
{
	glBindTexture(GL_TEXTURE_2D, m_handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	Format format;
	format.internalformat = GL_RGBA32F;
	format.width = width;
	format.height = height;
	format.format = GL_RGBA;
	format.type = GL_UNSIGNED_BYTE;

	Set(format);
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

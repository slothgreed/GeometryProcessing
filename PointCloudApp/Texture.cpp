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

void Texture::Set(const Format& format, unsigned char* data)
{
	Bind();
	glTexImage2D(
		format.target, format.level,
		format.internalformat,
		format.width, format.height,
		format.border, format.format,
		format.type, data);
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

Texture2D::Texture2D(const Sampler& sampler)
	:Texture2D()
{
	m_sampler = sampler;
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


Texture2D* Texture2D::Create(const Vector2i& resolute, const Vector4& color)
{
	auto pTexture = new Texture2D();
	unsigned char* pixelData = new unsigned char[resolute.x * resolute.y * 4];
	int index = 0;
	for (int i = 0; i < resolute.x; i++) {
		for (int j = 0; j < resolute.y; j++) {
			pixelData[index * 4] = color.x;
			pixelData[index * 4 + 1] = color.y;
			pixelData[index * 4 + 2] = color.z;
			pixelData[index * 4 + 3] = color.w;
			index++;
		}
	}

	pTexture->Build(resolute.x, resolute.y, pixelData);
	delete[] pixelData;
	return pTexture;
}

void Texture2D::Resize(int width, int height)
{
	if (m_format.width == width && m_format.height == height) { return; }
	m_format.width = width;
	m_format.height = height;
	Set(m_format, nullptr);
	OUTPUT_GLERROR;
}

Texture::Format Texture2D::CreateRGBA(int width, int height)
{
	Format format;
	format.internalformat = GL_RGBA32F;
	format.width = width;
	format.height = height;
	format.format = GL_RGBA;
	format.type = GL_UNSIGNED_BYTE;
	return format;
}
void Texture2D::Build(int width, int height, unsigned char* data)
{
	Bind();
	BindSampler(m_sampler);

	Set(CreateRGBA(width, height), data);
	OUTPUT_GLERROR;

}

void Texture2D::Build(int width, int height, const Sampler& sampler, unsigned char* data)
{
	m_sampler = sampler;
	Build(width, height, data);
}

void Texture2D::BindSampler(const Sampler& sampler)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.filter);
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

#include "Texture.h"
#include "TextureLoader.h"
#include "Utility.h"
namespace KI
{

Texture::Texture()
	:m_handle(0)
	,m_genMip(false)
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

void Texture::GetPixel(std::vector<unsigned char>& data)
{
	Bind();
	data.resize(m_format.width * m_format.height * GLUtil::GetFormatSize(m_format.format));
	glGetTexImage(m_format.target, m_format.level, m_format.format, m_format.type, data.data());
	OUTPUT_GLERROR;
}

int Texture::CalcMipmapLevel(const Vector2i& resolute)
{
	return 1 + static_cast<int>(std::floor(std::log2(std::max(resolute.x, resolute.y))));
}
Vector2i Texture::CalcMipmapResolute(const Vector2i& resolute, int mipLevel)
{
	return Vector2i(
		std::max(1, resolute.x >> mipLevel),
		std::max(1, resolute.y >> mipLevel));
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

Texture2D::~Texture2D()
{
	Delete();
}

Texture2D::Texture2D(const Format& format, const Sampler& sampler)
	:Texture2D()
{
	m_format = format;
	m_sampler = sampler;
	Bind();
	BindSampler(m_sampler);
}


void Texture2D::ClearMaxValue()
{
	if (m_format.format == GL_RED_INTEGER && m_format.type == GL_UNSIGNED_INT) {
		GLuint maxValue = 0x7F7FFFFF;
		glClearTexImage(m_handle, 0, m_format.format, m_format.type, &maxValue);
	} else {
		assert(0);
	}
}
void Texture2D::Clear(int value)
{
	glBindTexture(GL_TEXTURE_2D, m_handle);

	if (m_format.format == GL_RGBA && m_format.type == GL_FLOAT) {
		float values[4] = { (float)value,(float)value, (float)value, (float)value };
		glClearTexImage(m_handle, 0, m_format.format, m_format.type, &values);
	} else if (m_format.format == GL_RED && m_format.type == GL_UNSIGNED_INT) {
		auto values = (unsigned int)value;
		glClearTexImage(m_handle, 0, m_format.format, m_format.type, &values);
	} else if(m_format.format == GL_RGBA && m_format.type == GL_UNSIGNED_BYTE) {
		unsigned char values[4] = { (unsigned char)value,(unsigned char)value, (unsigned char)value, (unsigned char)value };
		glClearTexImage(m_handle, 0, m_format.format, m_format.type, &values);
	} else if (m_format.format == GL_RED && m_format.type == GL_FLOAT) {
		float values = (float)value;
		glClearTexImage(m_handle, 0, m_format.format, m_format.type, &values);
	} else {
		assert(0);
	}
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

Texture2D* Texture2D::Create(const Texture2D& texture)
{
	auto pTexture = new Texture2D();
	pTexture->Build(texture.Size().x, texture.Size().y);
	pTexture->Copy(texture);
	return pTexture;
}

void Texture2D::Copy(const Texture2D& texture)
{
	if (m_handle == -1) { return; }
	Resize(texture.Size().x, texture.Size().y);
	if (texture.GetFormat() != m_format) { assert(0); return; }
	glCopyImageSubData(
		texture.Handle(), texture.GetFormat().target, 0, 0, 0, 0,
		m_handle, m_format.target, 0, 0, 0, 0,
		texture.Size().x, texture.Size().y, 1);
	OUTPUT_GLERROR;
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
	format.internalformat = GL_RGBA8;
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

CubemapTexture::~CubemapTexture()
{
	Delete();
}

void CubemapTexture::Build(const Vector2i& resolute, bool mipmap)
{
	m_format.target = GL_TEXTURE_CUBE_MAP;
	m_format.format = GL_RGB;
	m_format.internalformat = GL_RGBA32F;
	m_format.level = 1;
	if (mipmap) {
		m_format.level = Texture::CalcMipmapLevel(resolute);
	}

	glBindTexture(m_format.target, m_handle);
	OUTPUT_GLERROR;

	for (size_t i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, m_format.internalformat, resolute.x, resolute.y, 0, m_format.format, GL_FLOAT, nullptr);
	}
	OUTPUT_GLERROR;

	if (mipmap) {
		glTexParameteri(m_format.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else {
		glTexParameteri(m_format.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	glTexParameteri(m_format.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_format.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_format.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_format.target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	OUTPUT_GLERROR;

	if (mipmap) {
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

}
void CubemapTexture::Build(const Vector<String>& path, bool mipmap)
{
	m_format.target = GL_TEXTURE_CUBE_MAP;
	m_format.format = GL_RGB;
	m_format.internalformat = GL_RGB;
	m_format.level = 1;
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);
	OUTPUT_GLERROR;

	for (size_t i = 0; i < path.size(); i++) {
		auto pixelData  = TextureLoader::LoadData(path[i], 0);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, m_format.internalformat, pixelData->width, pixelData->height, 0, m_format.format, GL_UNSIGNED_BYTE, pixelData->data);
		m_format.width = pixelData->width;
		m_format.height = pixelData->height;
	}
	OUTPUT_GLERROR;

	if (mipmap) {
		glTexParameteri(m_format.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else {
		glTexParameteri(m_format.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glTexParameteri(m_format.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_format.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_format.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_format.target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	OUTPUT_GLERROR;
	
	if (mipmap) {
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	OUTPUT_GLERROR;
	
}

void CubemapTexture::BuildArray(const Vector2i& resolute, bool mipmap)
{
	m_format.target = GL_TEXTURE_2D_ARRAY;
	m_format.internalformat = GL_RGBA32F;
	m_format.width = resolute.x;
	m_format.height = resolute.y;
	m_format.level = 1;
	if (mipmap) {
		m_format.level = Texture::CalcMipmapLevel(resolute);
	}

	glBindTexture(m_format.target, m_handle);
	OUTPUT_GLERROR;

	glTexStorage3D(m_format.target, m_format.level, m_format.internalformat, resolute.x, resolute.y, 6);
	OUTPUT_GLERROR;


	if (mipmap) {
		glTexParameteri(m_format.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else {
		glTexParameteri(m_format.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	glTexParameteri(m_format.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_format.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_format.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_format.target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	OUTPUT_GLERROR;

}

CubemapTexture* CubemapTexture::ConvertCubemap(const CubemapTexture* arr)
{
	auto pCubemap = new CubemapTexture();
	pCubemap->Build(Vector2i(arr->GetFormat().width, arr->GetFormat().height), arr->GetFormat().level > 1);
	assert(arr->GetFormat().internalformat == pCubemap->GetFormat().internalformat);
	for (int level = 0; level < arr->GetFormat().level; level++) {
		float width = std::max(1, arr->GetFormat().width >> level);
		float height = std::max(1, arr->GetFormat().height >> level);
		for (int i = 0; i < 6; i++) {
			glCopyImageSubData(
				arr->Handle(), GL_TEXTURE_2D_ARRAY, level,
				0, 0, i,
				pCubemap->Handle(), GL_TEXTURE_CUBE_MAP, level,
				0, 0, i,
				width, height, 1);
			OUTPUT_GLERROR;
		}
	}


	return pCubemap;
}

}

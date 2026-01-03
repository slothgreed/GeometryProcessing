#ifndef TEXTURE_H
#define	TEXTURE_H
#include "GLTypes.h"
namespace KI
{

class Texture
{
public:

	class Format
	{
	public:
		GLenum target;
		GLint level;
		GLint internalformat;
		GLsizei width;
		GLsizei height;
		GLsizei depth;
		GLint border;
		GLenum format;
		GLenum type;
		Format() :
			target(GL_TEXTURE_2D),
			level(0),
			internalformat(GL_RGBA),
			width(1),
			height(1),
			depth(1),
			border(0),
			format(GL_RGBA),
			type(GL_UNSIGNED_BYTE) {}

		~Format() {}

		bool operator==(const Format& rhs) const
		{
			return target == rhs.target &&
				level == rhs.level &&
				internalformat == rhs.internalformat &&
				width == rhs.width &&
				height == rhs.height &&
				border == rhs.border &&
				format == rhs.format &&
				type == rhs.type;
		}

		bool operator!=(const Format& rhs) const
		{
			return !(*this == rhs);
		}
	};

	class Sampler
	{
	public:
		enum WRAP
		{
			REPEAT = 0x2901,
		};

		enum FILTER
		{
			LINEAR = 0x2601,
			NEAREST = 0x2600
		};

		Sampler()
			: wrap(WRAP::REPEAT)
			, filter(FILTER::NEAREST)
			, mipmap(false)
		{
		}

		Sampler(FILTER _f)
			: wrap(WRAP::REPEAT)
			, filter(_f)
			, mipmap(false)
		{
		}

		void UseMipmap()
		{
			mipmap = true;
		}
		bool mipmap;
		WRAP wrap;
		FILTER filter;
	};

	Texture();
	~Texture();
	void Bind();
	void Delete();
	virtual TEXTURE_TYPE Type() const = 0;
	GLuint Handle() const { return m_handle; }
	Vector2i Size() const { return Vector2i(m_format.width, m_format.height); }
	const Format& GetFormat() const { return m_format; }
	void Set(const Format& format, unsigned char* data);
	void GetPixel(std::vector<unsigned char>& data);
	void UseMipmap() { m_sampler.UseMipmap(); }
	static int CalcMipmapLevel(const Vector2i& resolute);
	static Vector2i CalcMipmapResolute(const Vector2i& resolute, int mipLevel);
protected:
	bool m_genMip;
	Sampler m_sampler;
	Format m_format;
	GLuint m_handle;
};

class TextureBuffer : public Texture
{
public:
	TextureBuffer();
	~TextureBuffer();
	virtual TEXTURE_TYPE Type() const { return TEXTURE_BUFFER; }
	void Bind(int matrixId);
private:
};

class Texture2D : public Texture
{
public:
	Texture2D();
	Texture2D(const Format& format, const Sampler& sampler);

	~Texture2D();

	virtual TEXTURE_TYPE Type() const { return TEXTURE_2D; }
	static Texture2D* Create(const Vector2i& resolute, const Vector4& color);
	static Texture2D* Create(const Texture2D& texture);
	void Build(int width, int height);
	void Build(int width, int height, unsigned char* data);
	void Build(int width, int height, const Sampler& sampler, unsigned char* data);

	void BindSampler(const Sampler& sampler);
	static Texture::Format CreateRGBA(int width, int height);
	void Clear(int value);
	void ClearMaxValue();
	void Resize(int width, int height);
	void Copy(const Texture2D& texture);
private:

};

class Texture3D : public Texture
{
public:
	Texture3D();
	~Texture3D();
	virtual TEXTURE_TYPE Type() const { return TEXTURE_3D; }
	void Build(const Vector3i& size, Vector4* data);
	void Build(const Vector3i& size, unsigned short* data);

private:

};


class CubemapTexture : public Texture
{
public:
	CubemapTexture();
	~CubemapTexture();
	virtual TEXTURE_TYPE Type() const { return TEXTURE_CUBE_MAP; }

	void Build(const Vector2i& resolute, bool mipmap = true);
	void Build(const Vector<String>& path, bool mipmap = true);
	void BuildArray(const Vector2i& resolute, bool mipmap = true);

	static CubemapTexture* ConvertCubemap(const CubemapTexture* arr);

private:

};

}


#endif TEXTURE_H
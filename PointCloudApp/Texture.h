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
		GLint border;
		GLenum format;
		GLenum type;
		Format() :
			target(GL_TEXTURE_2D),
			level(0),
			internalformat(GL_RGBA),
			width(1),
			height(1),
			border(0),
			format(GL_RGBA),
			type(GL_UNSIGNED_BYTE) {}

		~Format() {}
	};

	Texture();
	~Texture();
	void Bind();
	void Delete();
	virtual TEXTURE_TYPE Type() const = 0;
	GLuint Handle() const { return m_handle; }
	Vector2 Size() const { return Vector2(m_format.width, m_format.height); }
	const Format& GetFormat() const { return m_format; }
	void Set(const Format& format);

protected:
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
	~Texture2D() {};

	virtual TEXTURE_TYPE Type() const { return TEXTURE_2D; }

	void Build(int width, int height);
	void Build(int width, int height, unsigned char* data);

	void Clear(int value);
	void Resize(int width, int height);
private:

};

class CubemapTexture : public Texture
{
public:
	CubemapTexture();
	~CubemapTexture() {};
	virtual TEXTURE_TYPE Type() const { return TEXTURE_CUBE_MAP; }

	void Build(const Vector<String>& path);
private:

};

}


#endif TEXTURE_H
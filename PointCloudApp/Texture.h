#ifndef TEXTURE_H
#define	TEXTURE_H
#include "GLTypes.h"
namespace KI
{

class Texture
{
public:
	Texture();
	~Texture();
	void Bind();
	void Delete();
	virtual TEXTURE_TYPE Type() const = 0;
	GLuint Handle() const { return m_handle; }
	const vec2& Size() const { return m_size; }
protected:
	vec2 m_size;
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
#ifndef TEXTURE_H
#define	TEXTURE_H
class Texture
{
public:
	Texture();
	~Texture();
	void Bind();
	void Delete();
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
	void Bind(int matrixId);
private:
};

class Texture2D : public Texture
{
public:
	Texture2D();
	~Texture2D() {};

	void Build(int width, int height);
	void Build(int width, int height, unsigned char* data);

	void Clear(int value);
	void Resize(int width, int height);

private:

};


#endif TEXTURE_H
#ifndef TEXTURE_H
#define	TEXTURE_H

class MatrixTexture
{
public:
	MatrixTexture();
	~MatrixTexture();

	void TextureBuffer(int matrixId);
	void Bind();

	void Delete();
private:
	GLenum m_type;
	GLuint m_Id;
};

#endif TEXTURE_H
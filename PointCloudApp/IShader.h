#ifndef ISHADER_H
#define ISHADER_H

#define ATTRIB_POSITION 0
#define ATTRIB_NORMAL   1
#define ATTRIB_COLOR   2

#include "GLBuffer.h"
namespace KI
{

class IShader
{
public:
	IShader()
		: m_programId(0) {};
	~IShader();

	virtual void Build() = 0;

	void Use();
	void UnUse();
	void Delete();
	GLuint BuildVertexFrag(const String& vert, const String& frag);

protected:
	GLuint Handle() const { return m_programId; };
	GLuint m_programId;
};


class IShadingShader : public IShader
{
public:

	enum class Type
	{
		Simple,
		VertexColor,
		Texture,
		GLTF
	};

	IShadingShader();
	virtual ~IShadingShader();

	virtual void Build();
	virtual Type GetType() = 0;
	virtual String GetVertexPath() = 0;
	virtual String GetFragmentPath() = 0;
	virtual void GetUniformLocation() = 0;
	virtual void SetViewProj(const Matrix4x4& value) = 0;
	virtual void SetModel(const Matrix4x4& value) { assert(0); };

	void SetVertexFormat(const VertexFormats& format);
	void SetVertexFormat(const VertexFormat& format);
	void DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer);
	void DrawArray(GLuint primitiveType, GLBuffer* pIndexBuffer);
private:

};

class IComputeShader : public IShader
{
public:
	IComputeShader() {};
	~IComputeShader() {};

	virtual String GetComputePath() = 0;
	
	virtual void Build();
	virtual void GetUniformLocation() = 0;

protected:
	glm::ivec3 m_dimension;
private:

};

class Texture;
class TextureShader : public IShadingShader
{
public:
	TextureShader() {};
	~TextureShader() {};
	virtual Type GetType() { return Type::Texture; }
	virtual void GetUniformLocation();
	virtual String GetVertexPath();
	virtual String GetFragmentPath();
	virtual void SetViewProj(const Matrix4x4& value) {};
	void BindTexture(const Texture& texture);
	void SetPosition(GLBuffer* pPosition);
	void SetTexture(GLBuffer* pTexture);
private:
	GLuint m_uniform;
};
}


#endif ISHADER_H
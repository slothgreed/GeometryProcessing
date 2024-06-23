#ifndef ISHADER_H
#define ISHADER_H

#define ATTRIB_POSITION 0
#define ATTRIB_NORMAL   1
#define ATTRIB_COLOR   2

#include "GLBuffer.h"
namespace KI
{

enum SHADER_PROGRAM
{
	SHADER_PROGRAM_VERTEX,
	SHADER_PROGRAM_GEOM,
	SHADER_PROGRAM_FRAG,
	SHADER_PROGRAM_MESH,
	SHADER_PROGRAM_COMPUTE
};

struct ShaderPath
{
	Vector<String> header;
	std::unordered_map<SHADER_PROGRAM, String> shader;
};


class IShader
{
public:
	IShader()
		: m_programId(0) {};
	~IShader();

	virtual void Build() = 0;
	virtual ShaderPath GetShaderPath() = 0;

	void Use();
	void UnUse();
	void Delete();

protected:
	String LoadHeaderCode(const Vector<String>& header);
	GLuint Handle() const { return m_programId; };
	GLuint m_programId;
};


class IShadingShader : public IShader
{
public:

	IShadingShader();
	virtual ~IShadingShader();

	virtual void Build();
	virtual void GetUniformLocation() = 0;
	virtual void SetViewProj(const Matrix4x4& value) = 0;
	virtual void SetModel(const Matrix4x4& value) { assert(0); };

	void SetVertexFormat(const VertexFormats& format);
	void SetVertexFormat(const VertexFormat& format);
	void DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer);
	void DrawArray(GLuint primitiveType, GLBuffer* pIndexBuffer);
private:
	GLuint BuildVertexFrag(const String& vert, const String& frag);

};

class IComputeShader : public IShader
{
public:
	IComputeShader() {};
	~IComputeShader() {};

	virtual void GetUniformLocation() = 0;

	virtual void Build();

protected:
	glm::ivec3 m_dimension;
private:

};

class IMeshShader : public IShader
{
public:
	IMeshShader() {};
	~IMeshShader() {};

	virtual void GetUniformLocation() {};
	virtual void Build();

private:

};


class Texture;
class TextureShader : public IShadingShader
{
public:
	TextureShader() {};
	~TextureShader() {};
	virtual ShaderPath GetShaderPath();
	virtual void GetUniformLocation();
	virtual void SetViewProj(const Matrix4x4& value) {};
	void BindTexture(const Texture& texture);
	void SetPosition(GLBuffer* pPosition);
	void SetTexture(GLBuffer* pTexture);
private:
	GLuint m_uniform;
};
}


#endif ISHADER_H
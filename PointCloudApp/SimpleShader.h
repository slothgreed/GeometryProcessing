#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H
#include "IShader.h"
#include "Texture.h"
namespace KI
{

class SimpleShader : public IShadingShader
{
public:
	SimpleShader();
	~SimpleShader();

	enum UNIFORM
	{
		MODEL,
		COLOR,
		NUM
	};

	virtual ShaderPath GetShaderPath() override;

	void GetUniformLocation();
	void SetCamera(const GLBuffer* pBuffer);
	void SetModel(const Matrix4x4& value);
	void SetColor(const Vector3& value);
	void SetPosition(GLBuffer* pBuffer);
private:

	GLuint m_uniform[UNIFORM::NUM];

};
class VertexColorShader : public IShadingShader
{
public:
	VertexColorShader();
	~VertexColorShader();

	enum UNIFORM
	{
		MODEL,
		NUM
	};

	virtual ShaderPath GetShaderPath() override;

	virtual void GetUniformLocation() override;
	virtual void SetCamera(const GLBuffer* pBuffer);
	virtual void SetModel(const Matrix4x4& value) override;
	void SetPosition(GLBuffer* pPosition);
	void SetColor(GLBuffer* pColor);

private:

	GLuint m_uniform[UNIFORM::NUM];

};

class PrimitiveColorShader : public IShadingShader
{
public:
	enum UNIFORM
	{
		MODEL,
		NUM
	};
	PrimitiveColorShader() {};
	~PrimitiveColorShader() {};
	virtual ShaderPath GetShaderPath() override;
	virtual void GetUniformLocation() override;
	virtual void SetCamera(const GLBuffer* pBuffer);
	virtual void SetModel(const Matrix4x4& value) override;
	void SetPosition(GLBuffer* pPosition);
	void SetColor(GLBuffer* pColor);

private:
	GLuint m_uniform[UNIFORM::NUM];

};

class InstancedPrimitiveShader : public IShadingShader
{
public:
	enum UNIFORM
	{
		COLOR,
		NUM
	};

	InstancedPrimitiveShader() {};
	~InstancedPrimitiveShader() {};
	virtual ShaderPath GetShaderPath() override;
	virtual void GetUniformLocation() override;
	virtual void SetCamera(const GLBuffer* pBuffer);
	void SetPosition(const GLBuffer* pPosition);
	void SetColor(const Vector3& color);
	void SetMatrixTexture(const TextureBuffer* pBuffer);
private:
	GLuint m_uniform[UNIFORM::NUM];
};

}
#endif SIMPLE_SHADER_H
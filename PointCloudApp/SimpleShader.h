#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H
#include "IShader.h"
namespace KI
{

class SimpleShader : public IShadingShader
{
public:
	SimpleShader();
	~SimpleShader();

	enum UNIFORM
	{
		VIEW_PROJ,
		MODEL,
		COLOR,
		NUM
	};

	virtual IShadingShader::Type GetType() { return IShadingShader::Type::Simple; }
	virtual ShaderPath GetShaderPath() override;

	void GetUniformLocation();
	void SetViewProj(const Matrix4x4& value);
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
		VIEW_PROJ,
		MODEL,
		NUM
	};

	virtual IShadingShader::Type GetType() { return IShadingShader::Type::VertexColor; }
	virtual ShaderPath GetShaderPath() override;

	virtual void GetUniformLocation() override;
	virtual void SetViewProj(const Matrix4x4& value) override;
	virtual void SetModel(const Matrix4x4& value) override;
	void SetPosition(GLBuffer* pPosition);
	void SetColor(GLBuffer* pColor);

private:

	GLuint m_uniform[UNIFORM::NUM];

};
}
#endif SIMPLE_SHADER_H
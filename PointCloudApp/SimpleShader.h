#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H
#include "IShader.h"
class SimpleShader : public IShader
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

	virtual IShader::Type GetType() { return IShader::Type::Simple; }
	virtual std::string GetVertexPath() override;
	virtual std::string GetFragmentPath() override;

	void GetUniformLocation();
	void SetViewProj(const mat4x4& value);
	void SetModel(const mat4x4& value);
	void SetColor(const vec3& value);
	virtual void SetupVertexAttribute();
private:

	GLuint m_uniform[UNIFORM::NUM];

};


#endif SIMPLE_SHADER_H
#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H
#include "IShader.h"
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
	virtual String GetVertexPath() override;
	virtual String GetFragmentPath() override;

	void GetUniformLocation();
	void SetViewProj(const mat4x4& value);
	void SetModel(const mat4x4& value);
	void SetColor(const vec3& value);
	virtual void SetupVertexAttribArray(GLBuffer* pBuffer);
private:

	GLuint m_uniform[UNIFORM::NUM];

};


#endif SIMPLE_SHADER_H
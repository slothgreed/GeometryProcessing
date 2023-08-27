#ifndef SHADER_H
#define SHADER_H
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
		NUM
	};

	virtual std::string GetVertexPath() override;
	virtual std::string GetFragmentPath() override;

	virtual void GetUniformLocation() override;
	virtual void SetViewProj(const mat4x4& value) override;
	virtual void SetModel(const mat4x4& value) override;
	virtual void SetupVertexAttribute();
private:

	GLuint m_uniform[UNIFORM::NUM];

};


#endif SHADER_H
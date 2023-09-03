#ifndef VERTEX_COLOR_SHADER_H
#define VERTEX_COLOR_SHADER_H
#include "IShader.h"
class VertexColorShader : public IShader
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

	virtual IShader::Type GetType() { return IShader::Type::VertexColor; }
	virtual std::string GetVertexPath() override;
	virtual std::string GetFragmentPath() override;

	virtual void GetUniformLocation() override;
	virtual void SetViewProj(const mat4x4& value) override;
	virtual void SetModel(const mat4x4& value) override;
	virtual void SetupVertexAttribute();
private:

	GLuint m_uniform[UNIFORM::NUM];

};


#endif POINT_CLOUD_SHADER_H
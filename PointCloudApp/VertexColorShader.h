#ifndef VERTEX_COLOR_SHADER_H
#define VERTEX_COLOR_SHADER_H
#include "IShader.h"
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
	virtual std::string GetVertexPath() override;
	virtual std::string GetFragmentPath() override;

	virtual void GetUniformLocation() override;
	virtual void SetViewProj(const mat4x4& value) override;
	virtual void SetModel(const mat4x4& value) override;
	void SetPosition(GLBuffer* pPosition);
	void SetColor(GLBuffer* pColor);

private:

	GLuint m_uniform[UNIFORM::NUM];

};


#endif VERTEX_COLOR_SHADER_H
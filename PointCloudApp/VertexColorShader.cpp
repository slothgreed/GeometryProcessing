#include "VertexColorShader.h"
#include "ShaderUtility.h"

using namespace std;
VertexColorShader::VertexColorShader()
{
}

VertexColorShader::~VertexColorShader()
{
	Delete();
}

std::string VertexColorShader::GetVertexPath()
{
	return  "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\vertexcolor.vert";
}
std::string VertexColorShader::GetFragmentPath()
{
	return  "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\vertexcolor.frag";
}
void VertexColorShader::GetUniformLocation()
{
	m_uniform[UNIFORM::VIEW_PROJ] = glGetUniformLocation(GetId(), "u_VP");
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(GetId(), "u_Model");
}

void VertexColorShader::SetViewProj(const mat4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::VIEW_PROJ], 1, GL_FALSE, &value[0][0]);
}

void VertexColorShader::SetModel(const mat4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}

void VertexColorShader::SetPosition(GLBuffer* pPosition)
{
	SetVertexAttribute(ATTRIB_POSITION, pPosition);
}
void VertexColorShader::SetColor(GLBuffer* pColor)
{
	SetVertexAttribute(ATTRIB_COLOR, pColor);
}



#include "SimpleShader.h"
#include "ShaderUtility.h"

using namespace std;
namespace KI
{

SimpleShader::SimpleShader()
{
}

SimpleShader::~SimpleShader()
{
}

ShaderPath SimpleShader::GetShaderPath()
{
	ShaderPath path;
	path.shader[SHADER_PROGRAM_VERTEX] = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\simple.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\simple.frag";
	return path;
}
void SimpleShader::GetUniformLocation()
{
	m_uniform[UNIFORM::VIEW_PROJ] = glGetUniformLocation(Handle(), "u_VP");
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
	m_uniform[UNIFORM::COLOR] = glGetUniformLocation(Handle(), "u_Color");
}

void SimpleShader::SetViewProj(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::VIEW_PROJ], 1, GL_FALSE, &value[0][0]);
}

void SimpleShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}

void SimpleShader::SetColor(const Vector3& value)
{
	glUniform3fv(m_uniform[UNIFORM::COLOR], 1, &value[0]);
}
void SimpleShader::SetPosition(GLBuffer* pBuffer)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pBuffer));
}

VertexColorShader::VertexColorShader()
{
}

VertexColorShader::~VertexColorShader()
{
}

ShaderPath VertexColorShader::GetShaderPath()
{
	ShaderPath path;
	path.shader[SHADER_PROGRAM_VERTEX] = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\vertexcolor.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\vertexcolor.frag";
	return path;
}
void VertexColorShader::GetUniformLocation()
{
	m_uniform[UNIFORM::VIEW_PROJ] = glGetUniformLocation(Handle(), "u_VP");
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
}

void VertexColorShader::SetViewProj(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::VIEW_PROJ], 1, GL_FALSE, &value[0][0]);
}

void VertexColorShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}

void VertexColorShader::SetPosition(GLBuffer* pPosition)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pPosition));

	glBindVertexBuffer(ATTRIB_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
	OUTPUT_GLERROR;
}
void VertexColorShader::SetColor(GLBuffer* pColor)
{
	SetVertexFormat(VertexFormat(ATTRIB_COLOR, pColor));

	glBindVertexBuffer(ATTRIB_COLOR, pColor->Handle(), 0, pColor->SizeOfData());
	OUTPUT_GLERROR;
}
}
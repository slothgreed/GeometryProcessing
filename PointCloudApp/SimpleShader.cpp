#include "SimpleShader.h"
#include "ShaderUtility.h"

using namespace std;
SimpleShader::SimpleShader()
{
}

SimpleShader::~SimpleShader()
{
	Delete();
}

String SimpleShader::GetVertexPath()
{
	return "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\simple.vert";
}
String SimpleShader::GetFragmentPath()
{
	return "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\simple.frag";
}
void SimpleShader::GetUniformLocation()
{
	m_uniform[UNIFORM::VIEW_PROJ] = glGetUniformLocation(GetId(), "u_VP");
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(GetId(), "u_Model");
	m_uniform[UNIFORM::COLOR] = glGetUniformLocation(GetId(), "u_Color");
}

void SimpleShader::SetViewProj(const mat4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::VIEW_PROJ], 1, GL_FALSE, &value[0][0]);
}

void SimpleShader::SetModel(const mat4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}

void SimpleShader::SetColor(const vec3& value)
{
	glUniform3fv(m_uniform[UNIFORM::COLOR], 1, &value[0]);
}
void SimpleShader::SetupVertexAttribArray(GLBuffer* pBuffer)
{
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribFormat(ATTRIB_POSITION, pBuffer->ComponentSize(),pBuffer->DataType(), GL_FALSE, 0);
	glBindVertexBuffer(0, pBuffer->Handle(), 0, pBuffer->SizeOfData());
	OUTPUT_GLERROR;
}


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
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "simple.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "simple.frag";
	return path;
}
void SimpleShader::GetUniformLocation()
{
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
	m_uniform[UNIFORM::COLOR] = glGetUniformLocation(Handle(), "u_Color");
}

void SimpleShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
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
	glBindVertexBuffer(ATTRIB_POSITION, pBuffer->Handle(), 0, pBuffer->SizeOfData());
	OUTPUT_GLERROR;
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
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "vertexcolor.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "vertexcolor.frag";
	return path;
}
void VertexColorShader::GetUniformLocation()
{
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
}

void VertexColorShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
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
	// 色がおかしいのはglVertexBindingのせい。コメントアウトすると正常になる。
	OUTPUT_GLERROR;
}



ShaderPath PrimitiveColorShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "primitivecolor.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "primitivecolor.frag";
	return path;
}
void PrimitiveColorShader::GetUniformLocation()
{
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
}

void PrimitiveColorShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
}

void PrimitiveColorShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}
void PrimitiveColorShader::SetPosition(GLBuffer* pPosition)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pPosition));
	glBindVertexBuffer(ATTRIB_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
}
void PrimitiveColorShader::SetColor(GLBuffer* pColor)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pColor->Handle());
}



ShaderPath InstancedPrimitiveShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "instance.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "instance.frag";
	return path;
}
void InstancedPrimitiveShader::GetUniformLocation()
{
	m_uniform[UNIFORM::COLOR] = glGetUniformLocation(Handle(), "u_Color");
}

void InstancedPrimitiveShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
}

void InstancedPrimitiveShader::SetColor(const Vector3& value)
{
	glUniform3fv(m_uniform[UNIFORM::COLOR], 1, &value[0]);
}
void InstancedPrimitiveShader::SetPosition(const GLBuffer* pPosition)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pPosition));
	glBindVertexBuffer(ATTRIB_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
}

void InstancedPrimitiveShader::SetMatrixTexture(const TextureBuffer* pBuffer)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, pBuffer->Handle());
}

}
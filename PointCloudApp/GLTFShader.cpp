#include "GLTFShader.h"
#include "GLTFStruct.h"
#include "Texture.h"
namespace KI
{

String GLTFShader::GetVertexPath()
{
	return "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\gltf.vert";
}
String GLTFShader::GetFragmentPath()
{
	return "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\gltf.frag";
}

void GLTFShader::BindBaseColor(const Texture& texture)
{
	assert(texture.Type() == TEXTURE_2D);
	glBindTextureUnit(0, texture.Handle());
	glUniform1i(m_uniform[UNIFORM::COLOR_TEXTURE], 0);
	OUTPUT_GLERROR;
}
void GLTFShader::BindNormal(const Texture& texture)
{
	assert(texture.Type() == TEXTURE_2D);
	glBindTextureUnit(1, texture.Handle());
	glUniform1i(m_uniform[UNIFORM::NORMAL_TEXTURE], 1);
	OUTPUT_GLERROR;
}
void GLTFShader::BindRoughness(const Texture& texture)
{
	assert(texture.Type() == TEXTURE_2D);
	glBindTextureUnit(2, texture.Handle());
	glUniform1i(m_uniform[UNIFORM::ROUGHNESS_TEXTURE], 2);
	OUTPUT_GLERROR;
}

void GLTFShader::BindBufferIndex(int matrix, int material)
{
	GLint vec[2] = { matrix, material };
	glUniform2iv(m_uniform[UNIFORM::SSBO_INDEX], 1, vec);
}

void GLTFShader::GetUniformLocation()
{
	m_uniform[UNIFORM::VIEW_PROJ] = glGetUniformLocation(Handle(), "u_VP");
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
	m_uniform[UNIFORM::SSBO_INDEX] = glGetUniformLocation(Handle(), "u_ssboIndex");
	m_uniform[UNIFORM::COLOR_TEXTURE] = glGetUniformLocation(Handle(), "u_colorTexture");
	m_uniform[UNIFORM::NORMAL_TEXTURE] = glGetUniformLocation(Handle(), "u_normalTexture");
	m_uniform[UNIFORM::ROUGHNESS_TEXTURE] = glGetUniformLocation(Handle(), "u_roughnessTexture");
}

void GLTFShader::SetViewProj(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::VIEW_PROJ], 1, GL_FALSE, &value[0][0]);
}

void GLTFShader::SetMaterialBuffer(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pBuffer->Handle());
}

void GLTFShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}
void GLTFShader::DrawElement(const GLTFPrimitive& primitive, GLuint dataType)
{
	glDrawElementsBaseVertex(primitive.meshType, primitive.drawNum, dataType, (void*)primitive.drawOffset, primitive.baseVertex);
	OUTPUT_GLERROR;
}


void GLTFShader::SetVertexBuffer(GLBuffer* pBuffer, const VertexFormats& format)
{
	glBindBuffer(GL_ARRAY_BUFFER, pBuffer->Handle());
	glBindVertexBuffer(0, pBuffer->Handle(), 0, pBuffer->SizeOfData());
	
	for (const auto& f : format) {
		SetVertexFormat(f);
	}
	glVertexAttribBinding(0, 0);
	glVertexAttribBinding(1, 0);
	glVertexAttribBinding(2, 0);
	glVertexAttribBinding(3, 0);
}
void GLTFShader::SetIndexBuffer(GLBuffer* pBuffer)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pBuffer->Handle());
}


}
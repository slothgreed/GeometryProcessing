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

void GLTFShader::BindMaterial(const GLTFMaterial& material)
{
	BindBaseColor(*material.pbr.baseTexture);
	BindNormal(*material.normalTexture);
	BindRoughness(*material.pbr.roughnessTexture);
}

void GLTFShader::GetUniformLocation()
{
	m_uniform[UNIFORM::VIEW_PROJ] = glGetUniformLocation(GetId(), "u_VP");
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(GetId(), "u_Model");
	m_uniform[UNIFORM::COLOR_TEXTURE] = glGetUniformLocation(GetId(), "u_colorTexture");
	m_uniform[UNIFORM::NORMAL_TEXTURE] = glGetUniformLocation(GetId(), "u_normalTexture");
	m_uniform[UNIFORM::ROUGHNESS_TEXTURE] = glGetUniformLocation(GetId(), "u_roughnessTexture");
}

void GLTFShader::SetViewProj(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::VIEW_PROJ], 1, GL_FALSE, &value[0][0]);
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
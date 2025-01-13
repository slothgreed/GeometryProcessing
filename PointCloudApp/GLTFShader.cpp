#include "GLTFShader.h"
#include "GLTFStruct.h"
#include "Texture.h"
namespace KI
{

ShaderPath GLTFShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.header.push_back("gltf\\gltf.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "gltf\\gltf.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "gltf\\gltf.frag";

	return path;
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
void GLTFShader::BindMetalRoughness(const Texture& texture)
{
	assert(texture.Type() == TEXTURE_2D);
	glBindTextureUnit(2, texture.Handle());
	glUniform1i(m_uniform[UNIFORM::METAL_ROUGHNESS_TEXTURE], 2);
	OUTPUT_GLERROR;
}

void GLTFShader::BindOcclusion(const Texture& texture)
{
	assert(texture.Type() == TEXTURE_2D);
	glBindTextureUnit(3, texture.Handle());
	glUniform1i(m_uniform[UNIFORM::OCCLUSION_TEXTURE], 3);
	OUTPUT_GLERROR;
}
void GLTFShader::BindEmissive(const Texture& texture)
{
	assert(texture.Type() == TEXTURE_2D);
	glBindTextureUnit(4, texture.Handle());
	glUniform1i(m_uniform[UNIFORM::EMISSIVE_TEXTURE], 4);
	OUTPUT_GLERROR;
}
void GLTFShader::BindBufferIndex(int matrix, int material)
{
	GLint vec[2] = { matrix, material };
	glUniform2iv(m_uniform[UNIFORM::SSBO_INDEX], 1, vec);
}

void GLTFShader::BindDebugView(int value)
{
	glUniform1i(m_uniform[UNIFORM::DEBUG_VIEW], value);
}

void GLTFShader::FetchUniformLocation()
{
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
	m_uniform[UNIFORM::SSBO_INDEX] = glGetUniformLocation(Handle(), "u_ssboIndex");
	m_uniform[UNIFORM::COLOR_TEXTURE] = glGetUniformLocation(Handle(), "u_colorTexture");
	m_uniform[UNIFORM::NORMAL_TEXTURE] = glGetUniformLocation(Handle(), "u_normalTexture");
	m_uniform[UNIFORM::METAL_ROUGHNESS_TEXTURE] = glGetUniformLocation(Handle(), "u_metalRoughnessTexture");
	m_uniform[UNIFORM::OCCLUSION_TEXTURE] = glGetUniformLocation(Handle(), "u_occlusionTexture");
	m_uniform[UNIFORM::EMISSIVE_TEXTURE] = glGetUniformLocation(Handle(), "u_emissiveTexture");
	m_uniform[UNIFORM::DEBUG_VIEW] = glGetUniformLocation(Handle(), "u_debugView");
}

void GLTFShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}

void GLTFShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
}

void GLTFShader::SetLight(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pBuffer->Handle());
}

void GLTFShader::SetMaterialBuffer(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pBuffer->Handle());
}

void GLTFShader::SetNodeBuffer(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pBuffer->Handle());
}

void GLTFShader::SetSkinBuffer(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, pBuffer->Handle());
}

void GLTFShader::DrawElement(const GLTFPrimitive& primitive, GLuint dataType)
{
	glDrawElementsBaseVertex(primitive.primitiveType, primitive.drawNum, dataType, (void*)primitive.drawOffset, primitive.baseVertex);
	OUTPUT_GLERROR;
}


void GLTFShader::SetVertexBuffer(GLBuffer* pBuffer, const VertexFormats& format)
{
	glBindBuffer(GL_ARRAY_BUFFER, pBuffer->Handle());
	glBindVertexBuffer(0, pBuffer->Handle(), 0, pBuffer->SizeOfData());
	
	for (const auto& f : format) {
		SetVertexFormat(f);
	}
}
void GLTFShader::SetIndexBuffer(GLBuffer* pBuffer)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pBuffer->Handle());
}


}
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
	BindTexture(m_uColorTexture, 0, texture);
}
void GLTFShader::BindNormal(const Texture& texture)
{
	BindTexture(m_uNormalTexture, 1, texture);
}
void GLTFShader::BindMetalRoughness(const Texture& texture)
{
	BindTexture(m_uMetalRoughnessTexture, 2, texture);
}

void GLTFShader::BindOcclusion(const Texture& texture)
{
	BindTexture(m_uOcclusionTexture, 3, texture);
}
void GLTFShader::BindEmissive(const Texture& texture)
{
	BindTexture(m_uEmissiveTexture, 4, texture);
}

void GLTFShader::BindBRDF(const Texture& texture)
{
	BindTexture(m_uBRDF, 5, texture);
}
void GLTFShader::BindIrradiance(const CubemapTexture& texture)
{
	BindCubemap(m_uIrradiance, 6, texture);
}
void GLTFShader::BindPrefilter(const CubemapTexture& texture)
{
	BindCubemap(m_uPrefilter, 7, texture);
}
void GLTFShader::BindBufferIndex(int matrix, int material)
{
	BindUniform(m_uSSBOIndex, Vector2i(matrix, material));
}

void GLTFShader::BindDebugView(int value)
{
	BindUniform(m_uDebugView, value);
}

void GLTFShader::FetchUniformLocation()
{
	m_uModel = GetUniformLocation("u_Model");
	m_uSSBOIndex = GetUniformLocation("u_ssboIndex");
	m_uColorTexture = GetUniformLocation("u_colorTexture");
	m_uNormalTexture = GetUniformLocation("u_normalTexture");
	m_uMetalRoughnessTexture = GetUniformLocation("u_metalRoughnessTexture");
	m_uOcclusionTexture = GetUniformLocation("u_occlusionTexture");
	m_uEmissiveTexture = GetUniformLocation("u_emissiveTexture");
	m_uDebugView = GetUniformLocation("u_debugView");
	m_uPrefilter = GetUniformLocation("u_prefilter");
	m_uIrradiance = GetUniformLocation("u_irradiance");
	m_uBRDF = GetUniformLocation("u_brdf");
}

void GLTFShader::SetModel(const Matrix4x4& value)
{
	BindUniform(m_uModel, value);
}

void GLTFShader::SetCamera(const GLBuffer* pBuffer)
{
	BindShaderStorage(0, pBuffer->Handle());
}

void GLTFShader::SetLight(const GLBuffer* pBuffer)
{
	BindShaderStorage(1, pBuffer->Handle());
}

void GLTFShader::SetMaterialBuffer(const GLBuffer* pBuffer)
{
	BindShaderStorage(3, pBuffer->Handle());
}

void GLTFShader::SetNodeBuffer(const GLBuffer* pBuffer)
{
	BindShaderStorage(4, pBuffer->Handle());
}

void GLTFShader::SetSkinBuffer(const GLBuffer* pBuffer)
{
	BindShaderStorage(5, pBuffer->Handle());
}

void GLTFShader::SetPBRResource(const GLBuffer* pBuffer)
{
	BindShaderStorage(6, pBuffer->Handle());
}

void GLTFShader::DrawElement(const GLTFPrimitive& primitive, GLuint dataType)
{
	glDrawElementsBaseVertex(primitive.primitiveType, primitive.drawNum, dataType, (void*)primitive.drawOffset, primitive.baseVertex);
	OUTPUT_GLERROR;
}


void GLTFShader::SetVertexBuffer(const GLBuffer* pBuffer, const VertexFormats& format)
{
	glBindBuffer(GL_ARRAY_BUFFER, pBuffer->Handle());
	glBindVertexBuffer(0, pBuffer->Handle(), 0, pBuffer->SizeOfData());
	
	for (const auto& f : format) {
		SetVertexFormat(f);
	}
}
void GLTFShader::SetIndexBuffer(const GLBuffer* pBuffer)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pBuffer->Handle());
}


}
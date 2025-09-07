#include "SimpleShader.h"
#include "ShaderUtility.h"
#include "PBR.h"
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
void SimpleShader::FetchUniformLocation()
{
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
	m_uniform[UNIFORM::COLOR] = glGetUniformLocation(Handle(), "u_Color");
}

void SimpleShader::SetCamera(const GLBuffer* pBuffer)
{
	BindShaderStorage(0, pBuffer->Handle());
}


void SimpleShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}

void SimpleShader::SetColor(const Vector3& value)
{
	glUniform3fv(m_uniform[UNIFORM::COLOR], 1, &value[0]);
}
void SimpleShader::SetPosition(const GLBuffer* pBuffer)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pBuffer));
	glBindVertexBuffer(ATTRIB_POSITION, pBuffer->Handle(), 0, pBuffer->SizeOfData());
	OUTPUT_GLERROR;
}


ShaderPath FaceShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.header.push_back("pbr\\pbr.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "face.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "face.frag";
	return path;
}

void FaceShader::SetNormal(const GLBuffer* pBuffer)
{
	SetVertexFormat(VertexFormat(ATTRIB_NORMAL, pBuffer));
	glBindVertexBuffer(ATTRIB_NORMAL, pBuffer->Handle(), 0, pBuffer->SizeOfData());
	OUTPUT_GLERROR;
}

void FaceShader::FetchUniformLocation()
{
	SimpleShader::FetchUniformLocation();
	m_uPrefilter = GetUniformLocation("u_prefilter");
	m_uIrradiance = GetUniformLocation("u_irradiance");
	m_uBRDF = GetUniformLocation("u_brdf");
}

void FaceShader::SetLight(const GLBuffer* pBuffer)
{
	BindShaderStorage(1, pBuffer->Handle());
}

void FaceShader::SetPBRResource(const PBRResource* pBuffer)
{
	BindShaderStorage(2, pBuffer->GetGlobalParam()->Handle());
	BindTexture(m_uBRDF, 5, *pBuffer->GetBRDFLUT());
	BindCubemap(m_uIrradiance, 6, *pBuffer->GetIrradiance());
	BindCubemap(m_uPrefilter, 7, *pBuffer->GetPrefiltered());
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
void VertexColorShader::FetchUniformLocation()
{
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
}

void VertexColorShader::SetCamera(const GLBuffer* pBuffer)
{
	BindShaderStorage(0, pBuffer->Handle());
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



ShaderPath PrimitiveColorShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "primitivecolor.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "primitivecolor.frag";
	return path;
}
void PrimitiveColorShader::FetchUniformLocation()
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
	BindShaderStorage(1, pColor->Handle());
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
void InstancedPrimitiveShader::FetchUniformLocation()
{
	m_uniform[UNIFORM::COLOR] = glGetUniformLocation(Handle(), "u_Color");
}

void InstancedPrimitiveShader::SetCamera(const GLBuffer* pBuffer)
{
	BindShaderStorage(0, pBuffer->Handle());
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

ShaderPath VertexVectorShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "vertexvector.vert";
	path.extension[SHADER_PROGRAM_GEOM].push_back("#extension GL_EXT_geometry_shader4 : enable\n");
	path.shader[SHADER_PROGRAM_GEOM] = "vertexvector.geom";
	path.shader[SHADER_PROGRAM_FRAG] = "vertexvector.frag";
	return path;
}
void VertexVectorShader::FetchUniformLocation()
{
	m_uniform[UNIFORM::COLOR] = GetUniformLocation("u_Color");
	m_uniform[UNIFORM::MODEL] = GetUniformLocation("u_Model");
	m_uniform[UNIFORM::LENGTH] = GetUniformLocation("u_Length");

}
void VertexVectorShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
	OUTPUT_GLERROR;
}
void VertexVectorShader::SetPosition(const GLBuffer* pPosition)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pPosition));
	glBindVertexBuffer(ATTRIB_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
	OUTPUT_GLERROR;
}

void VertexVectorShader::SetVector(const GLBuffer* pVector)
{
	SetVertexFormat(VertexFormat(ATTRIB_NORMAL, pVector));
	glBindVertexBuffer(ATTRIB_NORMAL, pVector->Handle(), 0, pVector->SizeOfData());
	OUTPUT_GLERROR;
}

void VertexVectorShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
	OUTPUT_GLERROR;
}

void VertexVectorShader::SetColor(const Vector4& value)
{
	glUniform4fv(m_uniform[UNIFORM::COLOR], 1, &value[0]);
	OUTPUT_GLERROR;
}
void VertexVectorShader::SetLength(float length)
{
	glUniform1f(m_uniform[UNIFORM::LENGTH], length);
	OUTPUT_GLERROR;
}

ShaderPath PointPickShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "pick.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "pick.frag";
	if (m_type == PickID) {
		path.extension[SHADER_PROGRAM_FRAG].push_back("#define PICK_BY_ID\n");
	} else if(m_type == PrimitiveID){
		path.extension[SHADER_PROGRAM_FRAG].push_back("#define PICK_BY_PRIMITIVE\n");
	}
	return path;
}
void PointPickShader::FetchUniformLocation()
{
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
	m_uniform[UNIFORM::PICKOFFSET] = glGetUniformLocation(Handle(), "u_PickOffset");

}
void PointPickShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
	OUTPUT_GLERROR;
}

void PointPickShader::SetPosition(VertexFormat format, const GLBuffer* pPosition)
{
	format.location = ATTRIB_POSITION;
	SetVertexFormat(format);
	glBindVertexBuffer(ATTRIB_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
}
void PointPickShader::SetPosition(const GLBuffer* pPosition)
{
	VertexFormat format = VertexFormat(ATTRIB_POSITION, pPosition);
	SetPosition(format, pPosition);
	OUTPUT_GLERROR;
}
void PointPickShader::SetModel(const Matrix4x4& value)
{
	BindUniform(m_uniform[UNIFORM::MODEL], value);
}

void PointPickShader::SetPickOffset(unsigned int offset)
{
	BindUniform(m_uniform[UNIFORM::PICKOFFSET], offset);
}

void PointPickShader::SetPickID(unsigned int pickID)
{
	// PickOffset‚ð‘ã‘Ö‚·‚éB
	BindUniform(m_uniform[UNIFORM::PICKOFFSET], pickID);
}


ShaderPath TextureShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "texture.vert";
	if (m_type == Type::VEC4) {
		path.shader[SHADER_PROGRAM_FRAG] = "texture.frag";
	} else if (m_type == Type::UINT) {
		path.shader[SHADER_PROGRAM_FRAG] = "textureUINT.frag";
	}

	return path;
}

void TextureShader::FetchUniformLocation()
{
	m_uniform[UNIFORM::TEXTURE] = GetUniformLocation("tex");
	m_uniform[UNIFORM::MODEL] = GetUniformLocation("u_Model");
}

void TextureShader::BindTexture(const Texture& texture)
{
	IShadingShader::BindTexture(m_uniform[UNIFORM::TEXTURE], 0, texture);
	/*
	glActiveTexture(GL_TEXTURE0);
	OUTPUT_GLERROR;
	glUniform1i(m_uniform[UNIFORM::TEXTURE], 0);
	OUTPUT_GLERROR;
	glBindTexture(GL_TEXTURE_2D, texture.Handle());
	OUTPUT_GLERROR;
	*/
}

void TextureShader::SetPosition(GLBuffer* pPosition)
{
	static const int ATTRIBUTE_POSITION = 0;
	SetVertexFormat(VertexFormat(ATTRIBUTE_POSITION, pPosition));

	glBindVertexBuffer(ATTRIBUTE_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
	OUTPUT_GLERROR;
}

void TextureShader::SetModel(const Matrix4x4& value)
{
	BindUniform(m_uniform[UNIFORM::MODEL], value);
}

void TextureShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
}


void TextureShader::SetTexcoord(GLBuffer* pTexture)
{
	static const int ATTRIBUTE_TEXCOORD = 1;
	SetVertexFormat(VertexFormat(ATTRIBUTE_TEXCOORD, pTexture));

	glBindVertexBuffer(ATTRIBUTE_TEXCOORD, pTexture->Handle(), 0, pTexture->SizeOfData());
	OUTPUT_GLERROR;

}




ShaderPath TextureViewShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "posteffect\\posteffect.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "posteffect\\viewtexture.frag";

	return path;
}

void TextureViewShader::FetchUniformLocation()
{
	m_uTexture = GetUniformLocation("tex");
}

void TextureViewShader::BindTexture(const Texture& texture)
{
	IShadingShader::BindTexture(m_uTexture, 0, texture);
}


ShaderPath CubemapViewShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");

	path.shader[SHADER_PROGRAM_VERTEX] = "posteffect\\posteffect.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "posteffect\\viewcubemap.frag";

	return path;
}

void CubemapViewShader::FetchUniformLocation()
{
	m_uCubeMap = GetUniformLocation("uCubemap");
	m_uMipmap = GetUniformLocation("uMipmap");
}

void CubemapViewShader::BindTexture(const CubemapTexture& texture)
{
	BindCubemap(m_uCubeMap, 0, texture);
}

void CubemapViewShader::BindMipmapLevel(int level)
{
	BindUniform(m_uMipmap, level);
}

}
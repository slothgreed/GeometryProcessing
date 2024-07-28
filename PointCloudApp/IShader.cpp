#include "IShader.h"
#include "ShaderUtility.h"
#include "Texture.h"
namespace KI
{
IShader::~IShader()
{
	Delete();
}
void IShader::Use()
{
	assert(m_programId != 0);
	glUseProgram(m_programId);
	OUTPUT_GLERROR;
}

void IShader::UnUse()
{
	glUseProgram(0);
	OUTPUT_GLERROR;
}
void IShader::Delete()
{
	if (m_programId == 0) {
		return;
	}
	glDeleteProgram(m_programId);
	m_programId = 0;
}


String IShader::LoadHeaderCode(const String& localPath, const Vector<String>& header)
{
	String headerCode;
	for (size_t i = 0; i < header.size(); i++) {
		headerCode += ShaderUtility::LoadFromFile(localPath + header[i]);
	}

	return headerCode;
}

void IShadingShader::SetVertexFormat(const VertexFormats& formats)
{
	for (const auto& format : formats) {
		SetVertexFormat(format);
	}
}

void IShadingShader::SetVertexFormat(const VertexFormat& format)
{
	glEnableVertexAttribArray(format.location);
	glVertexAttribFormat(format.location, format.componentSize, format.type, format.normalized, format.offset);
	glVertexAttribBinding(format.location, 0);
}

void IShadingShader::DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer->Handle());
	glDrawElements(primitiveType, pIndexBuffer->Num(), pIndexBuffer->DataType(), 0);
	OUTPUT_GLERROR;
}

void IShadingShader::DrawArray(GLuint primitiveType, GLBuffer* pPositionBuffer)
{
	DrawArray(primitiveType, pPositionBuffer->Num());
}

void IShadingShader::DrawArray(GLuint primitiveType, int count)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDrawArrays(primitiveType, 0, count);
	OUTPUT_GLERROR;
}


IShadingShader::IShadingShader()
{

}

IShadingShader::~IShadingShader()
{

}

String Join(const Vector<String>& strs)
{
	String ret;
	for (const auto& extension : strs) {
		ret += extension;
	}

	return ret;
}
void IShadingShader::Build()
{
	auto shaderPath = GetShaderPath();
	String localPath = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\";
	
	auto version = ShaderUtility::LoadFromFile(localPath + shaderPath.version);
	auto headerCode = LoadHeaderCode(localPath, shaderPath.header);
	auto vertexEx = Join(shaderPath.extension[SHADER_PROGRAM_VERTEX]);
	auto vertexCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_VERTEX]);
	auto fragEx = Join(shaderPath.extension[SHADER_PROGRAM_FRAG]);
	auto fragCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_FRAG]);


	vertexCode = version + vertexEx + headerCode + vertexCode;
	fragCode = version + fragEx + headerCode + fragCode;
	m_programId = IShadingShader::BuildVertexFrag(vertexCode, fragCode);


	GetUniformLocation();
	OUTPUT_GLERROR;
}

GLuint IShadingShader::BuildVertexFrag(const String& vert, const String& frag)
{
	GLuint vertexId = ShaderUtility::Compile(vert, GL_VERTEX_SHADER);
	GLuint fragId = ShaderUtility::Compile(frag, GL_FRAGMENT_SHADER);

	GLuint programId = ShaderUtility::Link(vertexId, fragId);

	glDeleteShader(vertexId);
	glDeleteShader(fragId);
	return programId;
}

void IMeshShader::Build()
{
	auto shaderPath = GetShaderPath();
	String localPath = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\";
	auto version = ShaderUtility::LoadFromFile(localPath + shaderPath.version);
	auto headerCode = LoadHeaderCode(localPath, shaderPath.header);

	auto meshEx = Join(shaderPath.extension[SHADER_PROGRAM_MESH]);
	auto fragEx = Join(shaderPath.extension[SHADER_PROGRAM_FRAG]);
	auto meshCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_MESH]);
	auto fragCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_FRAG]);
	GLuint meshId = ShaderUtility::Compile(version + meshEx + headerCode + meshCode, GL_MESH_SHADER_NV);
	GLuint fragId = ShaderUtility::Compile(version + fragEx + headerCode + fragCode, GL_FRAGMENT_SHADER);

	m_programId = ShaderUtility::Link(meshId, fragId);

	glDeleteShader(meshId);
	glDeleteShader(fragId);
	GetUniformLocation();
	return;
}
void IComputeShader::Build()
{
	String localPath = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\";
	auto shaderPath = GetShaderPath();
	auto version = ShaderUtility::LoadFromFile(localPath + shaderPath.version);
	auto computeEx = Join(shaderPath.extension[SHADER_PROGRAM_COMPUTE]);
	auto headerCode = LoadHeaderCode(localPath, shaderPath.header);
	auto computeCode = ShaderUtility::LoadFromFile(localPath + shaderPath.shader[SHADER_PROGRAM_COMPUTE]);

	GLuint computeId = ShaderUtility::Compile(version + computeEx + headerCode + computeCode, GL_COMPUTE_SHADER);

	m_programId = ShaderUtility::LinkCompute(computeId);

	GetUniformLocation();
	OUTPUT_GLERROR;
}


ShaderPath TextureShader::GetShaderPath()
{
	ShaderPath path;
	path.shader[SHADER_PROGRAM_VERTEX] = "texture.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "texture.frag";

	return path;
}


void TextureShader::GetUniformLocation()
{
	m_uniform = glGetUniformLocation(m_programId, "tex");
}

void TextureShader::BindTexture(const Texture& texture)
{
	glActiveTexture(GL_TEXTURE0);
	OUTPUT_GLERROR;
	glUniform1i(m_uniform, 0);
	OUTPUT_GLERROR;
	glBindTexture(GL_TEXTURE_2D, texture.Handle());
	OUTPUT_GLERROR;
}

void TextureShader::SetPosition(GLBuffer* pPosition)
{
	static const int ATTRIBUTE_POSITION = 0;
	SetVertexFormat(VertexFormat(ATTRIBUTE_POSITION, pPosition));

	glBindVertexBuffer(ATTRIBUTE_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
	OUTPUT_GLERROR;
}

void TextureShader::SetTexture(GLBuffer* pTexture)
{
	static const int ATTRIBUTE_TEXCOORD = 1;
	SetVertexFormat(VertexFormat(ATTRIBUTE_TEXCOORD, pTexture));

	glBindVertexBuffer(ATTRIBUTE_TEXCOORD, pTexture->Handle(), 0, pTexture->SizeOfData());
	OUTPUT_GLERROR;

}

}
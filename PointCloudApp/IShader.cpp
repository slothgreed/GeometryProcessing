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

String IShader::LoadHeaderCode()
{
	auto header = GetHeaderPath();
	String headerCode;
	for (size_t i = 0; i < header.size(); i++) {
		String code;
		ShaderUtility::LoadFromFile(header[i], code);
		headerCode += code;
	}

	return headerCode;
}

GLuint IShader::BuildVertexFrag(const String& vert, const String& frag)
{
	GLuint vertexId = ShaderUtility::Compile(vert, GL_VERTEX_SHADER);
	GLuint fragId = ShaderUtility::Compile(frag, GL_FRAGMENT_SHADER);

	GLuint programId = ShaderUtility::Link(vertexId, fragId);

	glDeleteShader(vertexId);
	glDeleteShader(fragId);
	return programId;
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
}

void IShadingShader::DrawElement(GLuint primitiveType, GLBuffer* pIndexBuffer)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer->Handle());
	glDrawElements(primitiveType, pIndexBuffer->Num(), pIndexBuffer->DataType(), 0);
	OUTPUT_GLERROR;
}

void IShadingShader::DrawArray(GLuint primitiveType, GLBuffer* pPositionBuffer)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDrawArrays(primitiveType, 0, pPositionBuffer->Num());
	OUTPUT_GLERROR;
}


IShadingShader::IShadingShader()
{

}

IShadingShader::~IShadingShader()
{

}

void IShadingShader::Build()
{

	auto headerCode = LoadHeaderCode();
	String vertexPath = GetVertexPath();
	String fragPath = GetFragmentPath();

	

	String vertexCode;
	String fragCode;
	ShaderUtility::LoadFromFile(vertexPath, vertexCode);
	ShaderUtility::LoadFromFile(fragPath, fragCode);

	vertexCode = headerCode + vertexCode;
	fragCode = headerCode + fragCode;
	m_programId = IShader::BuildVertexFrag(vertexCode, fragCode);


	GetUniformLocation();
	OUTPUT_GLERROR;
}

void IComputeShader::Build()
{
	auto headerCode = LoadHeaderCode();
	String computePath = GetComputePath();
	String computeCode;
	ShaderUtility::LoadFromFile(computePath, computeCode);
	GLuint computeId = ShaderUtility::Compile(headerCode + computeCode, GL_COMPUTE_SHADER);

	m_programId = ShaderUtility::LinkCompute(computeId);

	GetUniformLocation();
	OUTPUT_GLERROR;
}


String TextureShader::GetVertexPath()
{
	return "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\texture.vert";
}
String TextureShader::GetFragmentPath()
{
	return "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\texture.frag";
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
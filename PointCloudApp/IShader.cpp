#include "IShader.h"
#include "ShaderUtility.h"
#include "Texture.h"

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
	if (m_programId != 0) {
		glDeleteProgram(m_programId);
		m_programId = 0;
	}
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

void IShadingShader::SetVertexAttribute(int location, GLBuffer* pBuffer)
{
	glEnableVertexAttribArray(location);
	glVertexAttribFormat(location, pBuffer->ComponentSize(), pBuffer->DataType(), GL_FALSE, 0);
	OUTPUT_GLERROR;

	glBindVertexBuffer(location, pBuffer->Handle(), 0, pBuffer->SizeOfData());
	OUTPUT_GLERROR;
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
	String vertexPath = GetVertexPath();
	String fragPath = GetFragmentPath();

	String vertexCode;
	String fragCode;
	ShaderUtility::LoadFromFile(vertexPath, vertexCode);
	ShaderUtility::LoadFromFile(fragPath, fragCode);

	m_programId = IShader::BuildVertexFrag(vertexCode, fragCode);


	GetUniformLocation();
	OUTPUT_GLERROR;
}

void IComputeShader::Build()
{
	String computePath = GetComputePath();
	String computeCode;
	ShaderUtility::LoadFromFile(computePath, computeCode);
	GLuint computeId = ShaderUtility::Compile(computeCode, GL_COMPUTE_SHADER);

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
	SetVertexAttribute(ATTRIBUTE_POSITION, pPosition);
}

void TextureShader::SetTexture(GLBuffer* pTexture)
{
	static const int ATTRIBUTE_TEXCOORD = 1;
	SetVertexAttribute(ATTRIBUTE_TEXCOORD, pTexture);

}

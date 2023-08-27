#include "IShader.h"
#include "ShaderUtility.h"
IShader::IShader()
	:m_programId(0)
{

}

IShader::~IShader()
{

}

void IShader::Build()
{
	string vertexPath = GetVertexPath();
	string fragPath = GetFragmentPath();

	string vertexCode;
	string fragCode;
	ShaderUtility::LoadFromFile(vertexPath, vertexCode);
	ShaderUtility::LoadFromFile(fragPath, fragCode);
	GLuint vertexId = ShaderUtility::Compile(vertexCode, GL_VERTEX_SHADER);
	GLuint fragId = ShaderUtility::Compile(fragCode, GL_FRAGMENT_SHADER);

	m_programId = ShaderUtility::Link(vertexId, fragId);

	glDeleteShader(vertexId);
	glDeleteShader(fragId);

	GetUniformLocation();
}


void IShader::Use()
{
	assert(m_programId != 0);
	glUseProgram(m_programId);
	OUTPUT_GLERROR;
}
void IShader::Delete()
{
	if (m_programId != 0)
	{
		glDeleteProgram(m_programId);
		m_programId = 0;
	}
}

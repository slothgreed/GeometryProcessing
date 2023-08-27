#include "ShaderUtility.h"
#include <filesystem>
#include <fstream>
ShaderUtility::ShaderUtility()
{

}
ShaderUtility::~ShaderUtility()
{

}

GLuint ShaderUtility::Compile(const std::string& code, GLuint shaderType)
{
	GLuint id = glCreateShader(shaderType);

	const GLchar* sourceCode = code.c_str();
	GLint size = (GLint)code.size();
	glShaderSource(id, 1, &sourceCode, &size);

	glCompileShader(id);
	GLint result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		GLint maxLength = 0;
		// The maxLength includes the NULL character
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* errorLog = new GLchar[maxLength];
		glGetShaderInfoLog(id, maxLength, &maxLength, errorLog);
		assert(0);
		delete errorLog;
		errorLog = nullptr;
	}

	return id;
}
GLuint ShaderUtility::Link(GLuint vertexId, GLuint fragId)
{
	GLuint programId = glCreateProgram();

	if (vertexId != 0)
		glAttachShader(programId, vertexId);
	if (fragId != 0)
		glAttachShader(programId, fragId);

	if (vertexId != 0)
		glDeleteShader(vertexId);
	if (fragId != 0)
		glDeleteShader(fragId);

	glLinkProgram(programId);

	GLint result;
	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	if (result == GL_FALSE)
	{
		GLint maxLength = 0;
		// The maxLength includes the NULL character
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* errorLog = new GLchar[maxLength];
		glGetProgramInfoLog(programId, maxLength, &maxLength, errorLog);
		assert(0);
		delete errorLog;
		errorLog = nullptr;
	}

	return programId;
}
void ShaderUtility::LoadFromFile(const std::string& filePath, std::string& contents)
{
	ifstream ifs(filePath);
	string line;
	while ((getline(ifs, line)))
	{
		contents += line + "\n";
	}
}
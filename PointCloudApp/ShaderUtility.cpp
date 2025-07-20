#include "ShaderUtility.h"
#include <filesystem>
#include <fstream>
namespace KI
{
ShaderUtility::ShaderUtility()
{

}
ShaderUtility::~ShaderUtility()
{

}

GLuint ShaderUtility::Compile(const String& code, GLuint shaderType)
{
	if (code.empty()) { return 0; }
	GLuint id = glCreateShader(shaderType);
	OUTPUT_GLERROR;

	const GLchar* sourceCode = code.c_str();
	GLint size = (GLint)code.size();
	glShaderSource(id, 1, &sourceCode, &size);
	OUTPUT_GLERROR;

	glCompileShader(id);
	OUTPUT_GLERROR;
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
		delete[] errorLog;
		errorLog = nullptr;
	}

	OUTPUT_GLERROR;
	return id;
}
GLuint ShaderUtility::Link(GLuint vertexId, GLuint fragId)
{
	GLuint programId = glCreateProgram();

	if (vertexId != 0)
		glAttachShader(programId, vertexId);
	if (fragId != 0)
		glAttachShader(programId, fragId);

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
		String str = errorLog;
		printf(str.data());
		assert(0);
		delete[] errorLog;
		errorLog = nullptr;
	}

	OUTPUT_GLERROR;
	return programId;
}

GLuint ShaderUtility::Link(GLuint vertexId, GLuint geomId, GLuint fragId)
{
	GLuint programId = glCreateProgram();

	if (vertexId != 0) glAttachShader(programId, vertexId);
	if (geomId != 0) glAttachShader(programId, geomId);
	if (fragId != 0) glAttachShader(programId, fragId);

	glLinkProgram(programId);

	GLint result;
	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		GLint maxLength = 0;
		// The maxLength includes the NULL character
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* errorLog = new GLchar[maxLength];
		String str = errorLog;
		glGetProgramInfoLog(programId, maxLength, &maxLength, errorLog);
		assert(0);
		delete[] errorLog;
		errorLog = nullptr;
	}

	OUTPUT_GLERROR;
	return programId;
}

String ShaderUtility::LoadFromFile(const String& filePath)
{
	String contents;
	std::ifstream ifs(filePath);
	String line;
	while ((getline(ifs, line)))
	{
		contents += line + "\n";
	}

	return contents;
}

GLuint ShaderUtility::LinkCompute(GLuint computeId)
{
	GLuint programId = glCreateProgram();
	glAttachShader(programId, computeId);
	OUTPUT_GLERROR;
	glLinkProgram(programId);
	OUTPUT_GLERROR;
	GLint links;
	GLint result;
	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		GLint maxLength = 0;
		// The maxLength includes the NULL character
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* errorLog = new GLchar[maxLength];
		glGetProgramInfoLog(programId, maxLength, &maxLength, errorLog);
		assert(0);
		delete[] errorLog;
		errorLog = nullptr;
	}

	glDetachShader(programId, computeId);
	OUTPUT_GLERROR;

	return programId;
}
}

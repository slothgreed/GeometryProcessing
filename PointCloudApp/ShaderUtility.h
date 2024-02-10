#ifndef SHADER_UTILITY_H
#define SHADER_UTILITY_H

class ShaderUtility
{
public:
	ShaderUtility();
	~ShaderUtility();

	static GLuint Compile(const String& code, GLuint shaderType);
	static GLuint Link(GLuint vertexId, GLuint fragId);
	static GLuint LinkCompute(GLuint computeId);
	static void LoadFromFile(const String& filePath, String& contents);
private:

};


#endif
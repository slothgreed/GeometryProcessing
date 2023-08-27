#ifndef SHADER_UTILITY_H
#define SHADER_UTILITY_H

class ShaderUtility
{
public:
	ShaderUtility();
	~ShaderUtility();

	static GLuint Compile(const std::string& code, GLuint shaderType);
	static GLuint Link(GLuint vertexId, GLuint fragId);
	static void LoadFromFile(const std::string& filePath, std::string& contents);
private:

};


#endif
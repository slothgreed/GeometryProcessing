#ifndef MESH_SHADER_TEST_H
#define MESH_SHADER_TEST_H
#include "GLFWApp.h"
namespace KI
{

class MeshShaderTest : public GLFWApp
{
public:
	MeshShaderTest() {};
	~MeshShaderTest() {};

	virtual void Execute();
private:

	struct Shader : public IShader
	{

	};

};

}

#endif MESH_SHADER_TEST_H
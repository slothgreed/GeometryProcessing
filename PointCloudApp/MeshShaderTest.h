#ifndef MESH_SHADER_TEST_H
#define MESH_SHADER_TEST_H
#include "GLFWApp.h"
#include "IShader.h"
namespace KI
{

class MeshShaderTest : public GLFWApp
{
public:
	MeshShaderTest() {};
	~MeshShaderTest() {};

	virtual void Initialize();
	virtual void Execute();
	virtual void Finalize();
private:

	struct GpuObject
	{
		Shared<GLBuffer> meshlet;
		Shared<GLBuffer> position;
		Shared<GLBuffer> index;
	};

	class TriangleShader : public IMeshShader
	{
	public:
		TriangleShader() {};
		~TriangleShader() {};

		virtual ShaderPath GetShaderPath();
		void Draw(const GpuObject& quad);
	private:

	};

	GpuObject m_quad;

};

}

#endif MESH_SHADER_TEST_H
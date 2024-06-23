#ifndef COMPUTE_SHADER_TEST_H
#define	COMPUTE_SHADER_TEST_H
#include "TheApp.h"
#include "IShader.h"
namespace KI
{
class ComputeShaderTest : public TheApp
{
public:
	ComputeShaderTest();
	~ComputeShaderTest();
	void Initialize();
	void Execute();
	void Finalize();
private:
	class Shader : public IComputeShader
	{
	public:
		Shader() {};
		~Shader() {};

		virtual ShaderPath GetShaderPath() override;

		void Execute(const Vector<float>& in, Vector<float>& out);
		virtual void GetUniformLocation() override;
	private:
		int m_elementSize;
		GLuint m_uniform;
	};

};
}
#endif // COMPUTE_SHADER_TEST_H
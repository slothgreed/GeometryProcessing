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
		virtual Vector3i GetLocalThreadNum() const { return Vector3i(1024, 1, 1); }
		void Execute(const Vector<float>& in, Vector<float>& out);
		virtual void FetchUniformLocation() override;
	private:
		int m_elementSize;
		Vector3i m_dimension;
		GLuint m_uniform;
	};

	class PrefixSumShader : public IComputeShader
	{
	public:
		PrefixSumShader() {};
		virtual ~PrefixSumShader() {};

		virtual ShaderPath GetShaderPath() override;
		virtual Vector3i GetLocalThreadNum() const;
		void Execute(const Vector<int>& in, Vector<int>& out);
		virtual void FetchUniformLocation() override;
	private:
		GLuint m_num;
	};


};
}
#endif // COMPUTE_SHADER_TEST_H
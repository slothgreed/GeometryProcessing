#ifndef COMPUTE_SHADER_TEST_H
#define	COMPUTE_SHADER_TEST_H
#include "TheApp.h"
#include "IShader.h"
namespace KI
{

class ComputeShaderTest : public TheApp
{
public:
	ComputeShaderTest() {};
	~ComputeShaderTest() {};
	virtual void Initialize();
	virtual void Execute() = 0;
	virtual void Finalize();
	Vector<uint32_t> CreateData(int pattern, int num);

private:

};

class PrefixSumShaderPost : public IComputeShader
{
public:
	PrefixSumShaderPost() {};
	virtual ~PrefixSumShaderPost() {};
	virtual ShaderPath GetShaderPath() override;
	virtual Vector3i GetLocalThreadNum() const { return Vector3i(1024, 1, 1); }
	virtual void FetchUniformLocation() override;
	void Execute(Unique<GLBuffer>& outBuffer, Unique<GLBuffer>& blockSum);

	int m_num = 0;
};
class PrefixSumShader : public IComputeShader
{
public:
	PrefixSumShader();
	virtual ~PrefixSumShader() {}

	virtual ShaderPath GetShaderPath() override;
	virtual Vector3i GetLocalThreadNum() const { return Vector3i(1024, 1, 1); }
	virtual void FetchUniformLocation() override;
	Vector<uint32_t> Execute(const Vector<uint32_t>& in);
	void Execute(const Unique<GLBuffer>& inBuffer, Unique<GLBuffer>& outBuffer);

private:
	Unique<PrefixSumShaderPost> m_pPost;
	GLuint m_localPhase = 0;
	GLuint m_num = 0;
};
class PrefixSumTest : public ComputeShaderTest
{
public:
	PrefixSumTest() {};
	~PrefixSumTest() {};
	void Execute();
private:

	Vector<uint32_t> ExecuteCPU(const Vector<uint32_t>& in);
	Vector<uint32_t> ExecuteGPU(const Vector<uint32_t>& in);
	bool Validate(const Vector<uint32_t>& in, const Vector<uint32_t>& out);
	

};

class RadixSortTest : public ComputeShaderTest
{
public:
	RadixSortTest() {};
	~RadixSortTest() {};
	void Execute();

	class ScatterShader : public IComputeShader
	{
	public:
		ScatterShader() {};
		~ScatterShader() {};
		virtual ShaderPath GetShaderPath() override;
		virtual Vector3i GetLocalThreadNum() const { return Vector3i(1024, 1, 1); }
		virtual void FetchUniformLocation() override;
		void Execute(const Unique<GLBuffer>& in, const Unique<GLBuffer>& out, const Unique<GLBuffer>& offset, uint bitShift);
	private:
		GLuint m_uBitShift = 0;
		GLuint m_num = 0;
	};


private:
	Vector<uint32_t> ExecuteCPU(const Vector<uint32_t>& in);
	Vector<uint32_t> ExecuteGPU(const Vector<uint32_t>& in);
	bool Validate(const Vector<uint32_t>& in, const Vector<uint32_t>& out);
};

class HistogramShader : public IComputeShader
{
public:
	HistogramShader() {};
	virtual ~HistogramShader() {};

	virtual ShaderPath GetShaderPath() override;
	virtual Vector3i GetLocalThreadNum() const;
	Vector<uint32_t> Execute(const Vector<uint32_t>& in, uint bitShift, uint binNum);

	void Execute(const Unique<GLBuffer>& in, Unique<GLBuffer>& out, uint bitShift, uint binNum);
	virtual void FetchUniformLocation() override;
private:
	GLuint m_num = 0;
	GLuint m_bitShift = 0;
	GLuint m_binNum = 0;
};
class HistogramTest : public ComputeShaderTest	
{
public:
	HistogramTest() {};
	~HistogramTest() {};
	void Execute();
private:


};
}
#endif // COMPUTE_SHADER_TEST_H
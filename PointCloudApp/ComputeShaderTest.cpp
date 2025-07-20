#include <iostream>
#include "ComputeShaderTest.h"
#include "GLBuffer.h"
namespace KI
{
ComputeShaderTest::ComputeShaderTest()
{
}

ComputeShaderTest::~ComputeShaderTest()
{
}

void ComputeShaderTest::Initialize()
{
	if (glfwInit() == GL_FALSE) {
		std::cerr << "Can't initilize GLFW" << std::endl;
		return;
	}

	GLFWwindow* window = glfwCreateWindow(1024, 768, "PointCloudApp", NULL, NULL);
	if (window == NULL) {
		return;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		return;
	}
}

void ComputeShaderTest::Execute()
{	/*
	Vector<float> in(10000000);
	std::fill(in.begin(), in.end(), 0);
	auto shader = std::make_unique<ComputeShaderTest::Shader>();
	shader->Build();

	Vector<float> out;
	shader->Execute(in, out);
	*/
	Vector<int> in;
	for (int i = 0; i < 8; i++) {
		in.push_back(i);
	}

	auto shader = std::make_unique<ComputeShaderTest::PrefixSumShader>();
	shader->Build();
	Vector<int> out;
	shader->Execute(in, out);
}
void ComputeShaderTest::Finalize()
{
	glfwTerminate();
}



ShaderPath ComputeShaderTest::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "sample\\simple.comp";
	return path;
}

void ComputeShaderTest::Shader::Execute(const Vector<float>& in, Vector<float>& out)
{
	m_elementSize = 0;
	m_dimension = ivec3(256, 1, 1);
	auto buffer = std::make_unique<GLBuffer>();
	buffer->Create(in);

	Use();

	BindUniform(m_uniform, (uint)in.size());
	BindShaderStorage(3,buffer->Handle());
	Dispatch(m_elementSize / 256 + 1, m_dimension.y, m_dimension.z);
	UnUse();

	out.resize(in.size());
	buffer->GetBufferData(out);
}

void ComputeShaderTest::Shader::FetchUniformLocation()
{
	m_uniform = GetUniformLocation("u_elementSize");
}

ShaderPath ComputeShaderTest::PrefixSumShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "sample\\prefixsum.comp";
	return path;
}
Vector3i ComputeShaderTest::PrefixSumShader::GetLocalThreadNum() const 
{
	return Vector3i(1024, 1, 1);
}

void ComputeShaderTest::PrefixSumShader::Execute(const Vector<int>& in, Vector<int>& out)
{
	auto buffer = std::make_unique<GLBuffer>();
	buffer->Create(in);
	auto outBuffer = std::make_unique<GLBuffer>();
	out.resize(in.size());
	outBuffer->Create(out);

	Use();
	BindUniform(m_num, (int)in.size());
	BindShaderStorage(0, buffer->Handle());
	BindShaderStorage(1, outBuffer->Handle());
	Dispatch(GetDispatchNum1D(in.size()));
	BarrierSSBO();
	UnUse();

	outBuffer->GetBufferData(out);
}
void ComputeShaderTest::PrefixSumShader::FetchUniformLocation()
{
	m_num = GetUniformLocation("u_num");

}
}
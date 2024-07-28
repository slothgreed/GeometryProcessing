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
{
	Vector<float> in(10000000);
	std::fill(in.begin(), in.end(), 0);
	auto shader = std::make_unique<ComputeShaderTest::Shader>();
	shader->Build();


	Vector<float> out;
	shader->Execute(in, out);


}
void ComputeShaderTest::Finalize()
{
	glfwTerminate();
}



ShaderPath ComputeShaderTest::Shader::GetShaderPath()
{
	ShaderPath path;
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

	glUniform1ui(m_uniform, in.size());

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffer->Handle());
	glDispatchCompute(m_elementSize / 256 + 1, m_dimension.y, m_dimension.z);
	UnUse();

	out.resize(in.size());
	buffer->GetBufferData(out);
}

void ComputeShaderTest::Shader::GetUniformLocation()
{
	m_uniform = glGetUniformLocation(m_programId, "u_elementSize");
}
}
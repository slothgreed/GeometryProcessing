#include <iostream>
#include "ComputeShaderTest.h"
#include "GLBuffer.h"
#include "ComputeShaderSample.h"
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
	auto shader = std::make_unique<SimpleComputeShader>();
	shader->Build();


	Vector<float> out;
	shader->Execute(in, out);


}
void ComputeShaderTest::Finalize()
{
	glfwTerminate();
}
}
#include "MeshShaderTest.h"

namespace KI
{

void MeshShaderTest::Initialize()
{
	GLFWApp::Initialize();
	std::vector<glm::vec4> position;
	position.push_back(vec4(-0.5f, -0.5f, 0, 1.0f));
	position.push_back(vec4(-0.5f, 0.5f, 0, 1.0f));
	position.push_back(vec4(0.5f, 0.5f, 0, 1.0f));
	position.push_back(vec4(0.5f, -0.5f, 0, 1.0f));

	std::vector<int> index;
	index.push_back(0);
	index.push_back(1);
	index.push_back(2);

	index.push_back(2);
	index.push_back(3);
	index.push_back(0);

	m_quad.position = std::make_shared<GLBuffer>();
	m_quad.position->Create(position);
	m_quad.index = std::make_shared<GLBuffer>();
	m_quad.index->Create(index);


	std::vector<glm::ivec4> meshlet(2);
	meshlet[0].x = 0;	meshlet[0].y = 3;	meshlet[0].z = 0;	meshlet[0].w = 0;
	meshlet[1].x = 3;	meshlet[1].y = 3;	meshlet[1].z = 0;	meshlet[1].w = 0;

	m_quad.meshlet = std::make_shared<GLBuffer>();
	m_quad.meshlet->Create(meshlet);
}
void MeshShaderTest::Execute()
{
	auto pShader = std::make_unique<MeshShaderTest::TriangleShader>();
	pShader->Build();

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pShader->Draw(m_quad);

		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}
	glDeleteVertexArrays(1, &VertexArrayID);
}

void MeshShaderTest::Finalize()
{
	m_quad = GpuObject();
	GLFWApp::Finalize();
}
ShaderPath MeshShaderTest::TriangleShader::GetShaderPath()
{
	ShaderPath path;
	path.shader[SHADER_PROGRAM_MESH] = "sample\\trianglemesh.mesh";
	path.shader[SHADER_PROGRAM_FRAG] = "sample\\trianglemesh.frag";
	return path;
}

void MeshShaderTest::TriangleShader::Draw(const GpuObject& gpu)
{
	Use();
	BindShaderStorage(0, gpu.position->Handle());
	BindShaderStorage(1, gpu.index->Handle());
	BindShaderStorage(2, gpu.meshlet->Handle());
	DrawMeshTasks(0, gpu.meshlet->Num());
	UnUse();
}

}
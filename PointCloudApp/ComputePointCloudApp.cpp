#include <iostream>
#include "PointCloudIO.h"
#include "ComputePointCloudApp.h"
#include "RenderTextureNode.h"
#include "Texture.h"
#include "ComputeShaderSample.h"
#include "PointCloud.h"
#include "PointCloudComputeShader.h"
#include "MouseInput.h"
#include "Profiler.h"
namespace KI
{

void ComputePointCloudApp::ProcessMouseEvent(const MouseInput& input)
{
	m_pMouse->ApplyMouseInput(input);

	if (input.Event() == MOUSE_EVENT_WHEEL) {
		m_pCameraController->Wheel(*m_pMouse.get());
	} else if (input.Event() == MOUSE_EVENT_MOVE) {
		m_pCameraController->Move(*m_pMouse.get());
	}
}

void ComputePointCloudApp::ResizeEvent(int width, int height)
{
	if (m_pColorTexture) {
		m_pColorTexture->Resize(width, height);
	}
	glViewport(0, 0, width, height);
}
void ComputePointCloudApp::Execute()
{
	m_pColorTexture = std::make_shared<Texture2D>();
	m_pColorTexture->Build(512, 512);

	//CreateTextureComputeShader shader;
	//shader.Build();
	//shader.Execute(pTexture);

	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\cube.xyz")));
	//auto pPointCloud = Shared<PointCloud>(PointCloudIO::Create2D(1000, vec2(-100, -100), vec2(100, 100)));
	//auto pPointCloud = Shared<PointCloud>(PointCloudIO::Create2D(100, vec2(-100, -100), vec2(100, 100)));
	auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\cgModel\\pointCloud\\bildstein_station3_xyz_intensity_rgb.xyz")));
	BDB bdb;
	bdb.Apply(pPointCloud->GetBDB());
	m_pCamera->FitToBDB(bdb);

	int maxCountX, maxCountY, maxCountZ;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxCountX);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxCountY);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxCountZ);

	int maxSizeX, maxSizeY, maxSizeZ;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxSizeX);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxSizeY);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxSizeZ);

	auto pShader = std::make_unique<PointCloudComputeShader>(pPointCloud);
	pShader->Build();
	GPUProfiler profiler("Render");
	auto pNode = std::make_unique<RenderTextureNode>("Test", m_pColorTexture);
	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		profiler.Start();
		pShader->Execute(m_pCamera->Projection(), m_pCamera->ViewMatrix(), m_pColorTexture, nullptr);
		pNode->Draw(Matrix4x4(), Matrix4x4());
		profiler.Stop();

		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}
}

}

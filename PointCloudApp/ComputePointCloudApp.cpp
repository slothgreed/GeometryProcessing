#include <iostream>
#include "PointCloudIO.h"
#include "ComputePointCloudApp.h"
#include "RenderTextureNode.h"
#include "Texture.h"
#include "ComputeShaderSample.h"
#include "PointCloud.h"
#include "PointCloudComputeShader.h"
#include "MouseInput.h"
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
	Vector<Vector3> color(pPointCloud->Position().size(), Vector3(0.0f, 1.0f, 1.0f));
	//pPointCloud->SetColor(std::move(color));
	BDB bdb;
	bdb.Apply(pPointCloud->GetBDB());
	m_pCamera->FitToBDB(bdb);

	auto pShader = std::make_unique<PointCloudComputeShader>(pPointCloud);
	pShader->Build();
	auto pNode = std::make_unique<RenderTextureNode>("Test", m_pColorTexture);
	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pShader->Execute(m_pCamera->Projection(), m_pCamera->ViewMatrix(), m_pColorTexture, nullptr);
		pNode->Draw(Matrix4x4(), Matrix4x4());

		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}
}

}

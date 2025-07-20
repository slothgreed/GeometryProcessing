#include <iostream>
#include "PointCloudIO.h"
#include "ComputePointCloudApp.h"
#include "PostEffect.h"
#include "Texture.h"
#include "PointCloud.h"
#include "MouseInput.h"
#include "Profiler.h"
namespace KI
{

ShaderPath CreateTextureComputeShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "sample\\texture.comp";
	return path;
}
void CreateTextureComputeShader::Execute(const Shared<Texture>& pTexture)
{
	pTexture->Bind();
	Use();
	OUTPUT_GLERROR;
	glBindImageTexture(0, pTexture->Handle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	OUTPUT_GLERROR;
	glDispatchCompute(pTexture->Size().x, pTexture->Size().y, 1);
	OUTPUT_GLERROR;
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	OUTPUT_GLERROR;
	UnUse();
}

void CreateTextureComputeShader::FetchUniformLocation()
{

}


PointCloudComputeShader::PointCloudComputeShader(const Shared<PointCloud>& pPointCloud)
	:m_pPointCloud(pPointCloud)
{
	Initialize();
}

PointCloudComputeShader::~PointCloudComputeShader()
{
}

ShaderPath PointCloudComputeShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "pointcloud.comp";

	return path;
}

void PointCloudComputeShader::FetchUniformLocation()
{
	m_uniformVP = GetUniformLocation("u_VP");
}
void PointCloudComputeShader::Initialize()
{
	m_pPointBuffer = std::make_unique<GLBuffer>();
	m_pPointBuffer->Create(m_pPointCloud->CreatePositionColor4f());
}

void PointCloudComputeShader::Execute(const Matrix4x4& proj, const Matrix4x4& view, const Shared<Texture2D>& pColorBuffer, const Shared<Texture2D>& pDepthBuffer)
{
	pColorBuffer->Clear(0);
	if (pDepthBuffer) {
		pDepthBuffer->Clear(1);
	}

	Use();
	glBindImageTexture(0, pColorBuffer->Handle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	if (pDepthBuffer) {
		glBindImageTexture(1, pDepthBuffer->Handle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_pPointBuffer->Handle());

	Matrix4x4 vp = proj * view;
	glUniformMatrix4fv(m_uniformVP, 1, GL_FALSE, &vp[0][0]);
	glDispatchCompute(m_pPointCloud->Position().size() / 1024, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	OUTPUT_GLERROR;
	UnUse();
}


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
	m_pCameraController->FitToBDB(bdb);

	int maxCountX, maxCountY, maxCountZ;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxCountX);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxCountY);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxCountZ);

	int maxSizeX, maxSizeY, maxSizeZ;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxSizeX);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxSizeY);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxSizeZ);

	DrawContext context;
	auto pShader = std::make_unique<PointCloudComputeShader>(pPointCloud);
	pShader->Build();
	auto m_pResource = std::make_unique<RenderResource>();
	m_pResource->Build();

	GPUProfiler profiler("Render");
	auto pNode = std::make_unique<RenderTextureNode>();
	context.pResource = m_pResource.get();
	context.pResource->SetTexturePlane(pNode.get());
	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		profiler.Start();
		pShader->Execute(m_pCamera->Projection(), m_pCamera->ViewMatrix(), m_pColorTexture, nullptr);
		TextureDrawer::Execute(context, m_pColorTexture.get());
		profiler.Stop();

		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}
}

}

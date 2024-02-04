#include "PointCloudComputeShader.h"
#include "PointCloud.h"
#include "GLBuffer.h"
#include "Texture.h"
PointCloudComputeShader::PointCloudComputeShader(const std::shared_ptr<PointCloud>& pPointCloud)
	:m_pPointCloud(pPointCloud)
{
	Initialize();
}

PointCloudComputeShader::~PointCloudComputeShader()
{
}

std::string PointCloudComputeShader::GetComputePath()
{
	return "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\ComputeShader\\pointcloud.comp";

}
void PointCloudComputeShader::GetUniformLocation()
{
	m_uniformVP = glGetUniformLocation(m_programId, "u_VP");
}
void PointCloudComputeShader::Initialize()
{
	m_pPointBuffer = std::make_unique<GLBuffer>();
	m_pPointBuffer->Create(m_pPointCloud->CreatePosition4f());

	m_pColorBuffer = std::make_unique<GLBuffer>();
	m_pColorBuffer->Create(m_pPointCloud->CreateColor4f());

}

void PointCloudComputeShader::Execute(const mat4x4& proj, const mat4x4& view, const std::shared_ptr<Texture2D>& pColorBuffer, const std::shared_ptr<Texture2D>& pDepthBuffer)
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_pColorBuffer->Handle());

	mat4x4 vp = proj * view;
	glUniformMatrix4fv(m_uniformVP, 1, GL_FALSE, &vp[0][0]);
	glDispatchCompute(m_pPointCloud->Position().size(), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	OUTPUT_GLERROR;
	UnUse();
}
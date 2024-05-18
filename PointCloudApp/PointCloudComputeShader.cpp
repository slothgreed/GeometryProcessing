#include "PointCloudComputeShader.h"
#include "PointCloud.h"
#include "GLBuffer.h"
#include "Texture.h"

namespace KI
{

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
	path.shader[SHADER_PROGRAM_COMPUTE] = "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\ComputeShader\\pointcloud.comp";

	return path;
}

void PointCloudComputeShader::GetUniformLocation()
{
	m_uniformVP = glGetUniformLocation(m_programId, "u_VP");
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
	glDispatchCompute(m_pPointCloud->Position().size() /1024, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	OUTPUT_GLERROR;
	UnUse();
}
}

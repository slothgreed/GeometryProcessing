#include "RenderResource.h"
#include "Camera.h"
#include "Light.h"

namespace KI
{

void GLContext::SetupStatus(const GLStatus& status)
{
	if (status.backCull) {
		EnableCullFace();
	} else {
		DisableCullFace();
	}

	SetPointSize(status.pointSize);
	SetLineWidth(status.lineWidth);
}

void GLContext::SetViewport(const Vector2& size)
{
	glViewport(0, 0, size.x, size.y);
}
void GLContext::EnablePolygonOffset(int factor, int units)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(factor, units);
}

void GLContext::EnableCullFace()
{
	glEnable(GL_CULL_FACE);
}
void GLContext::DisableCullFace()
{
	glDisable(GL_CULL_FACE);
}
void GLContext::EnableDepth()
{
	glEnable(GL_DEPTH_TEST);
}
void GLContext::DisableDepth()
{
	glDisable(GL_DEPTH_TEST);
}
void GLContext::DisablePolygonOffset()
{
	glDisable(GL_POLYGON_OFFSET_FILL);
}
void GLContext::SetPointSize(float value)
{
	if (m_cache.pointSize == value) { return; }
	if (value < 0) { return; }
	glPointSize(value);
	OUTPUT_GLERROR;
	m_cache.pointSize = value;
}

void GLContext::SetLineWidth(float value)
{
	if (m_cache.lineWidth == value) { return; }
	if (value < 0) { return; }
	glLineWidth(value);
	OUTPUT_GLERROR;
	m_cache.lineWidth = value;
}

void GLContext::SetupPick()
{
	SetPointSize(8.0f);
	SetLineWidth(7.0f);
}
void GLContext::SetupShading()
{
	SetPointSize(8.0f);
	SetLineWidth(3.0f);
}
void RenderResource::Build()
{
	m_pShaderTable.Build();
};
void RenderResource::UpdateCamera()
{
	if (!m_pCameraGpu) {
		m_pCameraGpu = new GLBuffer();
		m_pCameraGpu->Create(1, 256);
	}

	struct CameraGPU
	{
		Matrix4x4 view;
		Matrix4x4 proj;
		Matrix4x4 vp;
		Vector4 eye;
		float padding[12];
	};

	CameraGPU gpu;
	gpu.view = m_pCamera->ViewMatrix();
	gpu.proj = m_pCamera->Projection();
	gpu.vp = m_pCamera->Projection() * m_pCamera->ViewMatrix();
	gpu.eye = Vector4(m_pCamera->Eye(), 1.0f);
	m_pCameraGpu->BufferSubData(0, 1, sizeof(CameraGPU), &gpu);
}

void RenderResource::UpdateLight()
{
	if (!m_pLightGpu) {
		m_pLightGpu = new GLBuffer();
		m_pLightGpu->Create(1, 256);
	}
	struct LightGPU
	{
		Vector4 color;
		Vector4 direction;
		float padding[56];
	};

	LightGPU gpu;
	gpu.color = Vector4(m_pLight->GetColor(), 1.0f);
	gpu.direction = Vector4(glm::normalize(m_pCamera->Direction()), 1.0f);
	m_pLightGpu->BufferSubData(0, 1, sizeof(LightGPU), &gpu);
}

void RenderResource::Finalize()
{
	delete m_pCameraGpu;
	m_pCameraGpu = nullptr;

	delete m_pLightGpu;
	m_pLightGpu = nullptr;
}
void RenderResource::Resize(const Vector2& size)
{
	if (m_pRenderTarget) { m_pRenderTarget->Resize(size); }
}

void GLContext::PushRenderTarget(RenderTarget* pTarget)
{
	m_pRenderTargetStack.push(pTarget);

	pTarget->Bind();
}
void GLContext::PopRenderTarget()
{
	if (m_pRenderTargetStack.empty()) { return; }

	auto pTarget = m_pRenderTargetStack.top();
	m_pRenderTargetStack.pop();

	pTarget->UnBind();
}

void RenderResource::InitComputeTarget()
{
	if (m_pComputeColorTarget) { m_pComputeColorTarget->ClearMaxValue(); }
	if (m_pComputeDepthTarget) { m_pComputeDepthTarget->ClearMaxValue(); }
}
}
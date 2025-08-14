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

void GLContext::SetViewport(const Vector2i& size)
{
	glViewport(0, 0, size.x, size.y);
	viewportSize = size;
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
	m_pPBR = new PBRResource();
	m_pComputeColorTarget = new GLBuffer();
	m_pComputeDepthTarget = new GLBuffer();
	m_pPostEffectTarget = RenderTarget::CreatePostEffectTarget(Vector2i(1, 1));
	m_pTmpComputeTarget = RenderTarget::CreateForwardTarget(Vector2i(1, 1));
	m_pTmpPostEffectTarget = RenderTarget::CreatePostEffectTarget(Vector2i(1, 1));

};
void RenderResource::UpdateCamera()
{
	struct CameraGPU
	{
		Matrix4x4 view;
		Matrix4x4 proj;
		Matrix4x4 vp;
		Matrix4x4 invVP;
		Vector4 eye;
		Vector4 center;
		Vector2 viewSize;
		float padding[2];
	};

	if (!m_pCameraGpu) {
		m_pCameraGpu = new GLBuffer();
		m_pCameraGpu->Create(1, sizeof(CameraGPU));
	}


	CameraGPU gpu;
	gpu.view = m_pCamera->ViewMatrix();
	gpu.proj = m_pCamera->Projection();
	gpu.vp = m_pCamera->Projection() * m_pCamera->ViewMatrix();
	gpu.invVP = glm::inverse(gpu.vp);
	gpu.eye = Vector4(m_pCamera->Eye(), 1.0f);
	gpu.center = Vector4(m_pCamera->Center(), 1.0f);
	gpu.viewSize = m_pCamera->ViewSize();
	m_pCameraGpu->BufferSubData(0, 1, sizeof(CameraGPU), &gpu);
}

void RenderResource::UpdatePBR()
{
	struct PBRGpu
	{
		PBRGpu()
			: prefilteredMaxMip(0)
			, exposure(1.0f)
			, pad1(0.0f), pad2(0.0f)
		{
		}
			
		int prefilteredMaxMip;
		float exposure;
		float pad1;
		float pad2;
	};

	if (!m_pPBRGpu) {
		m_pPBRGpu = new GLBuffer();
		m_pPBRGpu->Create(1, sizeof(PBRGpu));
	}


	PBRGpu gpu;
	gpu.prefilteredMaxMip = GetPBR()->GetPrefiltered()->GetFormat().level;
	gpu.exposure = 5.0f;
	m_pPBRGpu->BufferSubData(0, 1, sizeof(PBRGpu), &gpu);
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
	RELEASE_INSTANCE(m_pCameraGpu);
	RELEASE_INSTANCE(m_pLightGpu);
	RELEASE_INSTANCE(m_pComputeColorTarget);
	RELEASE_INSTANCE(m_pComputeDepthTarget);
	RELEASE_INSTANCE(m_pTmpComputeTarget);
	RELEASE_INSTANCE(m_pPostEffectTarget);
	RELEASE_INSTANCE(m_pPBRGpu);
	RELEASE_INSTANCE(m_pPBR);
}


void GLContext::PushRenderTarget(RenderTarget* pTarget, int drawTargetNum)
{
	RenderTargetStack stack;
	stack.pRenderTarget = pTarget;
	stack.drawTargetNum = drawTargetNum;
	m_pRenderTargetStack.push(stack);
	pTarget->Bind(drawTargetNum);
}

void GLContext::ColorMask(bool value)
{
	if (value) {
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	} else {
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	}
}
void GLContext::PopRenderTarget()
{
	if (m_pRenderTargetStack.empty()) {
		FrameBuffer::UnBind();
		return;
	}
	m_pRenderTargetStack.pop();
	if (m_pRenderTargetStack.empty()) {
		FrameBuffer::UnBind();
		return;
	}

	auto pTarget = m_pRenderTargetStack.top();

	pTarget.pRenderTarget->Bind(pTarget.drawTargetNum);
}

void RenderResource::InitRenderTarget(const Vector2& size)
{
	if (m_pRenderTarget) {
		m_pRenderTarget->Resize(size);
		m_pRenderTarget->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	if (m_pComputeColorTarget) {
		m_pComputeColorTarget->Resize(size.x * size.y, sizeof(unsigned int));
		m_pComputeColorTarget->SetData(0x7F7FFFFF);
	}
	if (m_pComputeDepthTarget) {
		m_pComputeDepthTarget->Resize(size.x * size.y, sizeof(unsigned int));
		m_pComputeDepthTarget->SetData(0x7F7FFFFF);
	}

	m_pTmpComputeTarget->Resize(size);
	m_pTmpPostEffectTarget->Resize(size);
	m_pPostEffectTarget->Resize(size);

}
}
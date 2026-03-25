#include "RenderResource.h"
#include "Camera.h"
#include "Light.h"

namespace KI
{
Viewport Viewport::Create(const Vector2i& windowSize, const Vector2i& ratioSize, Viewport::Anchor anchor)
{
	int w = windowSize.x / ratioSize.x;
	int h = windowSize.y / ratioSize.y;

	int x = 0;
	int y = 0;
	switch (anchor) {
	case Viewport::Anchor::TopLeft:
		x = 0;
		y = windowSize.y - h;
		break;
	case Anchor::TopRight:
		x = windowSize.x - w;
		y = windowSize.y - h;
		break;
	case Anchor::BottomLeft:
		x = 0;
		y = 0;
		break;
	case Anchor::BottomRight:
		x = windowSize.x - w;
		y = 0;
		break;
	}
	return Vector4i(x, y, w, h);
}

std::array<Viewport, 2> Viewport::SplitHorizontal(const Viewport& viewport)
{
	const auto& v = viewport.Get();
	const int leftW = v.z / 2;
	const int rightW = v.z - leftW;

	return
	{
		Viewport(Vector4i(v.x,         v.y, leftW,  v.w)),
		Viewport(Vector4i(v.x + leftW, v.y, rightW, v.w))
	};
}
std::array<Viewport, 2> Viewport::SplitVertical(const Viewport& viewport)
{
	const auto& v = viewport.Get();
	const int bottomH = v.w / 2;
	const int topH = v.w - bottomH;

	return
	{
		Viewport(Vector4i(v.x, v.y + bottomH, v.z, topH)),
		Viewport(Vector4i(v.x, v.y,           v.z, bottomH))
	};
}


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

void GLContext::SetWindowSize(const Vector2i& size)
{
	m_windowSize = size;
	SetViewport(m_windowSize);
}

void GLContext::Clear(GLuint clear)
{
	glClear(clear);
}

Viewport GLContext::CreateViewport(const Vector2i& ratioSize, Viewport::Anchor anchor) const
{
	return Viewport::Create(m_windowSize, ratioSize, anchor);
}
void GLContext::SetViewport(const Viewport& viewport)
{
	const auto& value = viewport.Get();
	glViewport(value.x, value.y, value.z, value.w);
}

void GLContext::SetViewportFullWindow()
{
	SetViewport(m_windowSize);
}

void GLContext::EnableScissor(const Viewport& value)
{
	glEnable(GL_SCISSOR_TEST);
	const auto& scissor = value.Get();
	glScissor(scissor.x, scissor.y, scissor.z, scissor.w);
}
void GLContext::DisableScissor()
{
	glDisable(GL_SCISSOR_TEST);
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

void GLContext::EnablePolygonWire()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}
void GLContext::EnablePolygonFill()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
	m_pDebugTarget = RenderTarget::CreateForwardTarget(Vector2i(1, 1));
	m_pPostEffectTarget = RenderTarget::CreatePostEffectTarget(Vector2i(1, 1));
	m_pTmpComputeTarget = RenderTarget::CreateForwardTarget(Vector2i(1, 1));
	m_pTmpPostEffectTarget = RenderTarget::CreatePostEffectTarget(Vector2i(1, 1));

};
void RenderResource::UpdateCamera()
{
	if (!m_pCameraGpu) {
		m_pCameraGpu = new GLBuffer();
		m_pCameraGpu->Create(1, sizeof(ShaderLayout::Camera));

		m_p2DCameraGpu = new GLBuffer();
		m_p2DCameraGpu->Create(1, sizeof(ShaderLayout::Camera));
	}

	{
		ShaderLayout::Camera gpu;
		gpu.view = m_pCamera->ViewMatrix();
		gpu.proj = m_pCamera->Projection();
		gpu.vp = m_pCamera->Projection() * m_pCamera->ViewMatrix();
		gpu.invVP = glm::inverse(gpu.vp);
		gpu.eye = Vector4(m_pCamera->Eye(), 1.0f);
		gpu.center = Vector4(m_pCamera->Center(), 1.0f);
		gpu.viewSize = m_pCamera->ViewSize();
		auto frustum = m_pCamera->CreateFrustum().plane;
		for (int i = 0; i < 6; i++) {
			gpu.frustum[i] = frustum[i];
		}
		m_pCameraGpu->BufferSubData(0, 1, sizeof(ShaderLayout::Camera), &gpu);
	}
	{
		ShaderLayout::Camera gpu;
		gpu.view = Matrix4x4(1);
		gpu.proj = Camera::Create2DProj(m_pCamera->ViewSize());
		gpu.vp = gpu.proj * gpu.view;
		gpu.invVP = glm::inverse(gpu.vp);
		gpu.eye = Vector4(m_pCamera->Eye(), 1.0f);
		gpu.center = Vector4(m_pCamera->Center(), 1.0f);
		gpu.viewSize = m_pCamera->ViewSize();
		m_p2DCameraGpu->BufferSubData(0, 1, sizeof(ShaderLayout::Camera), &gpu);
	}
}

void RenderResource::UpdateDebugCamera(const Camera& camera)
{
	if (!m_pDebugCameraGpu) {
		m_pDebugCameraGpu = new GLBuffer();
		m_pDebugCameraGpu->Create(1, sizeof(ShaderLayout::Camera));
	}
	ShaderLayout::Camera gpu;
	gpu.view = camera.ViewMatrix();
	gpu.proj = camera.Projection();
	gpu.vp = camera.Projection() * camera.ViewMatrix();
	gpu.invVP = glm::inverse(gpu.vp);
	gpu.eye = Vector4(camera.Eye(), 1.0f);
	gpu.center = Vector4(camera.Center(), 1.0f);
	gpu.viewSize = camera.ViewSize();
	auto frustum = camera.CreateFrustum().plane;
	for (int i = 0; i < 6; i++) {
		gpu.frustum[i] = frustum[i];
	}
	m_pDebugCameraGpu->BufferSubData(0, 1, sizeof(ShaderLayout::Camera), &gpu);
}

void RenderResource::UpdatePBR()
{
	if (m_pPBR) { m_pPBR->Update(); }
}

void RenderResource::UpdateLight()
{
	if (!m_pLightGpu) {
		m_pLightGpu = new GLBuffer();
		m_pLightGpu->Create(1, 256);
	}

	ShaderLayout::Light gpu;
	gpu.color = Vector4(m_pLight->GetColor(), 1.0f);
	gpu.direction = Vector4(glm::normalize(m_pCamera->Direction()), 1.0f);
	m_pLightGpu->BufferSubData(0, 1, sizeof(ShaderLayout::Light), &gpu);
}

void RenderResource::Finalize()
{
	RELEASE_INSTANCE(m_pDebugCameraGpu);
	RELEASE_INSTANCE(m_pCameraGpu);
	RELEASE_INSTANCE(m_pLightGpu);
	RELEASE_INSTANCE(m_pComputeColorTarget);
	RELEASE_INSTANCE(m_pComputeDepthTarget);
	RELEASE_INSTANCE(m_pDebugTarget);
	RELEASE_INSTANCE(m_pTmpComputeTarget);
	RELEASE_INSTANCE(m_pPostEffectTarget);
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

	m_pDebugTarget->Resize(size);
	m_pTmpComputeTarget->Resize(size);
	m_pTmpPostEffectTarget->Resize(size);
	m_pPostEffectTarget->Resize(size);

}
}
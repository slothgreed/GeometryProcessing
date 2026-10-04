#include "GL/RenderResource.h"
#include "Renderer/Camera/Camera.h"
#include "Renderer/ClusteredLightCuller.h"
#include "Renderer/Light.h"
#include "Renderer/Shader/PostEffect.h"
#include "Utility/BDB.h"
#include "Utility/Utility.h"
namespace KI
{
void RenderResource::Build()
{
	m_pShaderTable.Build();
	m_pPBR = new PBRResource();
	m_pComputeColorTarget = new GLBuffer();
	m_pComputeDepthTarget = new GLBuffer();
	m_pComputeAccumTarget = new GLBuffer();
	m_pDebugTarget = RenderTarget::CreateForwardTarget(Vector2i(1, 1));
	m_pPostEffectTarget = RenderTarget::CreatePostEffectTarget(Vector2i(1, 1));
	m_pTmpComputeTarget = RenderTarget::CreateForwardTarget(Vector2i(1, 1));
	m_pTmpPostEffectTarget = RenderTarget::CreatePostEffectTarget(Vector2i(1, 1));
	m_pTexturePlane = new TexturePlane();
	m_pClusteredLightResource = new ClusteredLightResource();

};
void RenderResource::UpdateCamera()
{
	if (!m_pCameraGpu) {
		m_pCameraGpu = new GLBuffer();
		m_pCameraGpu->Create(1, sizeof(ShaderLayout::Camera));

		m_p2DCameraGpu = new GLBuffer();
		m_p2DCameraGpu->Create(1, sizeof(ShaderLayout::Camera));
	}

	UpdateCamera(*m_pCamera);
}
void RenderResource::UpdateCamera(const Camera& camera)
{
	{
		ShaderLayout::Camera gpu;
		gpu.view = camera.ViewMatrix();
		gpu.proj = camera.Projection();
		gpu.vp = camera.Projection() * camera.ViewMatrix();
		gpu.invVP = glm::inverse(gpu.vp);
		gpu.eye = Vector4(camera.Eye(), 1.0f);
		gpu.center = Vector4(camera.Center(), 1.0f);
		gpu.viewSize = camera.ViewSize();
		gpu._near = camera.GetNear();
		gpu._far = camera.GetFar();
		auto frustum = camera.CreateFrustum().plane;
		for (int i = 0; i < 6; i++) {
			gpu.frustum[i] = frustum[i];
		}
		m_pCameraGpu->BufferSubData(0, 1, sizeof(ShaderLayout::Camera), &gpu);
	}
	{
		ShaderLayout::Camera gpu;
		gpu.view = Matrix4x4(1);
		gpu.proj = Camera::Create2DProj(camera.ViewSize());
		gpu.vp = gpu.proj * gpu.view;
		gpu.invVP = glm::inverse(gpu.vp);
		gpu.eye = Vector4(camera.Eye(), 1.0f);
		gpu.center = Vector4(camera.Center(), 1.0f);
		gpu.viewSize = camera.ViewSize();
		gpu._near = camera.GetNear();
		gpu._far = camera.GetFar();
		m_p2DCameraGpu->BufferSubData(0, 1, sizeof(ShaderLayout::Camera), &gpu);
	}
}
void RenderResource::UpdateCamera(const BDB& bdb)
{
	UpdateCamera(Camera::FitToBDB(*m_pCamera, bdb));
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

void RenderResource::UpdateLight(const Vector2i& windowSize)
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
	RELEASE_INSTANCE(m_pComputeAccumTarget);
	RELEASE_INSTANCE(m_pDebugTarget);
	RELEASE_INSTANCE(m_pTmpComputeTarget);
	RELEASE_INSTANCE(m_pPostEffectTarget);
	RELEASE_INSTANCE(m_pPBR);
	RELEASE_INSTANCE(m_pTexturePlane);
	RELEASE_INSTANCE(m_pClusteredLightResource);
}



void RenderResource::InitRenderTarget(const Vector2& size)
{
	if (m_pRenderTarget) {
		m_pRenderTarget->Resize(size);
		m_pRenderTarget->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	if (m_pComputeColorTarget) {
		m_pComputeColorTarget->Resize(size.x * size.y, sizeof(unsigned int));
		// floatMax
		m_pComputeColorTarget->SetData(0x7F7FFFFF);
	}

	if (m_pComputeAccumTarget) {
		m_pComputeAccumTarget->Resize(size.x * size.y, sizeof(unsigned int) * 4);
		m_pComputeAccumTarget->SetData(0);
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

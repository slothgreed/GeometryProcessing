#include "RenderResource.h"
#include "Camera.h"
#include "Light.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace KI
{

void GLContext::SetViewport(const Vector2& size)
{
	glViewport(0, 0, size.x, size.y);
}
void GLContext::EnablePolygonOffset(int factor, int units)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(factor, units);
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
	if (m_pointSize == value) { return; }
	glPointSize(value);
	m_pointSize = value;
}

void GLContext::SetLineWidth(float value)
{
	if (m_lineWidth == value) { return; }
	glLineWidth(value);
	m_lineWidth = value;
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
		Matrix4x4 vp;
		Vector4 eye;
		float padding[44];
	};

	CameraGPU gpu;
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
	gpu.direction = Vector4(m_pLight->GetDirection(), 1.0f);
	m_pLightGpu->BufferSubData(0, 1, sizeof(LightGPU), &gpu);
}
void RenderResource::Finalize()
{
	delete m_pCameraGpu;
	m_pCameraGpu = nullptr;

	delete m_pLightGpu;
	m_pLightGpu = nullptr;
}

}
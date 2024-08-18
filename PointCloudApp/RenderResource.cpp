#include "RenderResource.h"
#include "Camera.h"
#include "Light.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace KI
{

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

void RenderResource::ShowUI()
{
	ImGui::Text(
		"Eye:(%lf,%lf,%lf)\nCenter:(%lf,%lf,%lf)\nUp:(%lf,%lf,%lf)\n",
		m_pCamera->Eye().x, m_pCamera->Eye().y, m_pCamera->Eye().z,
		m_pCamera->Center().x, m_pCamera->Center().y, m_pCamera->Center().z,
		m_pCamera->Up().x, m_pCamera->Up().y, m_pCamera->Up().z);

	Vector3 color = m_pLight->GetColor();
	if (ImGui::ColorEdit3("Light Color", &color[0])) {
		m_pLight->SetColor(color);
	}

	ImGui::Text("Direction:(%lf, %lf, %lf)\n",	m_pLight->GetDirection().x, m_pLight->GetDirection().y, m_pLight->GetDirection().z);
}
}
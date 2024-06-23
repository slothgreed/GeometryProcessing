#include "RenderResource.h"
#include "Camera.h"
namespace KI
{

void RenderResource::Build() {
	m_pShaderTable.Build();
	if (!m_pCamera) {
		m_pCamera = new GLBuffer();
		m_pCamera->Create(1, 256);
	}
};
void RenderResource::UpdateCamera(const Camera* pCamera)
{
	struct CameraGPU
	{
		Matrix4x4 vp;
		float padding[48];
	};

	CameraGPU gpu;
	gpu.vp = pCamera->Projection() * pCamera->ViewMatrix();
	m_pCamera->BufferSubData(0, 1, sizeof(CameraGPU), &gpu);
}

void RenderResource::Finalize()
{
	delete m_pCamera;
	m_pCamera = nullptr;
}
}
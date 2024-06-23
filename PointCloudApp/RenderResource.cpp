#include "RenderResource.h"
#include "Camera.h"
namespace KI
{

void RenderResource::UpdateCamera(const Camera* pCamera)
{
	struct CameraGPU
	{
		Matrix4x4 vp;
		float padding[240];
	};

	CameraGPU gpu;
	gpu.vp = pCamera->Projection() * pCamera->ViewMatrix();
	if (!m_pCamera) {
		m_pCamera = new GLBuffer();
		m_pCamera->Create(DATA_UNKNOWN, 1, sizeof(CameraGPU), &gpu);
	} else {
		m_pCamera->BufferSubData(0, 1, sizeof(CameraGPU), &gpu);
	}
}

void RenderResource::Finalize()
{
	delete m_pCamera;
	m_pCamera = nullptr;
}
}
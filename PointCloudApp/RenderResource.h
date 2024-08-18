#ifndef RENDER_RESOURCE_H
#define RENDER_RESOURCE_H
#include "ShaderTable.h"

namespace KI
{
class Camera;
class Light;
class RenderResource
{
public:
	RenderResource()
		: m_pCameraGpu(nullptr)
		, m_pLightGpu(nullptr) {};
	~RenderResource() {};
	void Build();

	void SetCamera(const Shared<Camera>& pCamera) { m_pCamera = pCamera; }
	void SetLight(const Shared<Light>& pLight) { m_pLight = pLight; }

	void Finalize();
	ShaderTable* GetShaderTable() { return &m_pShaderTable; };
	const ShaderTable* GetShaderTable() const { return &m_pShaderTable; };
	const GLBuffer* GetCameraBuffer() const { return m_pCameraGpu; }
	const GLBuffer* GetLightBuffer() const { return m_pLightGpu; }

	void ShowUI();
	void UpdateLight();
	void UpdateCamera();
private:
	
	Shared<Camera> m_pCamera;
	Shared<Light> m_pLight;
	GLBuffer* m_pCameraGpu;
	GLBuffer* m_pLightGpu;
	ShaderTable m_pShaderTable;
};
}

#endif RENDER_RESOURCE_H
#ifndef RENDER_RESOURCE_H
#define RENDER_RESOURCE_H
#include "ShaderTable.h"

namespace KI
{
class GLContext
{
public:
	GLContext()
		:m_pointSize(-1)
		, m_lineWidth(-1)
	{
	}
	~GLContext() {};

	void EnablePolygonOffset(int factor, int units);
	void DisablePolygonOffset();
	void EnableDepth();
	void DisableDepth();
	void SetViewport(const Vector2& size);
	void SetPointSize(float value);
	void SetLineWidth(float value);

	void SetupPick();
	void SetupShading();
private:
	Vector2 viewportSize;
	float m_pointSize;
	float m_lineWidth;
};

class Camera;
class Light;
class RenderResource
{
public:
	RenderResource()
		: m_pContext(std::make_unique<GLContext>())
		, m_pCameraGpu(nullptr)
		, m_pLightGpu(nullptr) {};
	~RenderResource() {};
	void Build();

	void SetCamera(const Shared<Camera>& pCamera) { m_pCamera = pCamera; }
	void SetLight(const Shared<Light>& pLight) { m_pLight = pLight; }
	GLContext* GL() { return m_pContext.get(); }
	const Shared<Camera>& GetCamera() const { return m_pCamera; }
	const Shared<Light>& GetLight() const { return m_pLight; }
	void Finalize();
	ShaderTable* GetShaderTable() { return &m_pShaderTable; };
	const ShaderTable* GetShaderTable() const { return &m_pShaderTable; };
	const GLBuffer* GetCameraBuffer() const { return m_pCameraGpu; }
	const GLBuffer* GetLightBuffer() const { return m_pLightGpu; }
	void UpdateLight();
	void UpdateCamera();
private:
	Unique<GLContext> m_pContext;
	Shared<Camera> m_pCamera;
	Shared<Light> m_pLight;
	GLBuffer* m_pCameraGpu;
	GLBuffer* m_pLightGpu;
	ShaderTable m_pShaderTable;
};
}

#endif RENDER_RESOURCE_H
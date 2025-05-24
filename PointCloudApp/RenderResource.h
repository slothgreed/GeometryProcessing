#ifndef RENDER_RESOURCE_H
#define RENDER_RESOURCE_H
#include "ShaderTable.h"
#include "Texture.h"
#include "RenderTarget.h"
namespace KI
{
struct GLStatus
{
	friend class GLContext;
	GLStatus()
		:backCull(true)
		,pointSize(-1.0f)
		,lineWidth(-1.0f)
	{
	}
	
	void SetPointSize(float size) { lineWidth = size; }
	void SetLineWidth(float size) { pointSize = size; }
	void SetBackCull(bool value) { backCull = value; }
private:
	float pointSize;
	float lineWidth;
	bool backCull;
};
class GLContext
{
public:
	GLContext()
	{
	}
	~GLContext() {};

	void SetupStatus(const GLStatus& status);
	void EnablePolygonOffset(int factor, int units);
	void DisablePolygonOffset();
	void EnableDepth();
	void DisableDepth();
	void EnableCullFace();
	void DisableCullFace();
	void SetViewport(const Vector2& size);
	void SetPointSize(float value);
	void SetLineWidth(float value);

	void SetupPick();
	void SetupShading();

	void PushRenderTarget(RenderTarget* pTarget);
	void PopRenderTarget();
private:
	Vector2 viewportSize;
	GLStatus m_cache;
	std::stack<RenderTarget*> m_pRenderTargetStack;
};

class Camera;
class Light;
class RenderResource
{
public:
	RenderResource()
		: m_pContext(std::make_unique<GLContext>())
		, m_pCameraGpu(nullptr)
		, m_pLightGpu(nullptr)
		, m_pComputeColorTarget(nullptr)
		, m_pComputeDepthTarget(nullptr)
		, m_pRenderTarget(nullptr)
		, m_pTexturePlane(nullptr) {};
	~RenderResource() {};
	void Build();

	void SetComputeColorTarget(Texture2D* pTarget) { m_pComputeColorTarget = pTarget; }
	void SetComputeDepthTarget(Texture2D* pTarget) { m_pComputeDepthTarget = pTarget; }
	void SetRenderTarget(RenderTarget* pRenderTarget) { m_pRenderTarget = pRenderTarget; }
	void SetMainCamera(const Shared<Camera>& pCamera) { m_pCamera = pCamera; }
	void SetLight(const Shared<Light>& pLight) { m_pLight = pLight; }
	GLContext* GL() { return m_pContext.get(); }
	const Shared<Camera>& GetCamera() const { return m_pCamera; }
	const Shared<Light>& GetLight() const { return m_pLight; }
	void Finalize();
	ShaderTable* GetShaderTable() { return &m_pShaderTable; };
	const ShaderTable* GetShaderTable() const { return &m_pShaderTable; };
	const GLBuffer* GetCameraBuffer() const { return m_pCameraGpu; }
	const GLBuffer* GetLightBuffer() const { return m_pLightGpu; }
	const RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }
	const Texture2D* GetComputeColorTarget() const { return m_pComputeColorTarget; }
	const Texture2D* GetComputeDepthTarget() const { return m_pComputeDepthTarget; }
	void UpdateLight();
	void UpdateCamera();
	void Resize(const Vector2& size);
	void InitComputeTarget();
	void SetTexturePlane(RenderTextureNode* pPlane) { m_pTexturePlane = pPlane; };
	const RenderTextureNode* GetTexturePlane() const { return m_pTexturePlane; }
private:
	Unique<GLContext> m_pContext;
	Shared<Camera> m_pCamera;
	Shared<Light> m_pLight;
	GLBuffer* m_pCameraGpu;
	GLBuffer* m_pLightGpu;
	Texture2D* m_pComputeColorTarget;
	Texture2D* m_pComputeDepthTarget;
	RenderTarget* m_pRenderTarget;
	ShaderTable m_pShaderTable;
	RenderTextureNode* m_pTexturePlane;
};
}

#endif RENDER_RESOURCE_H
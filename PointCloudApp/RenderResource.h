#ifndef RENDER_RESOURCE_H
#define RENDER_RESOURCE_H
#include "ShaderTable.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "PBR.h"
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
	void SetViewport(const Vector2i& size);
	void SetPointSize(float value);
	void SetLineWidth(float value);

	void SetupPick();
	void SetupShading();

	void PushRenderTarget(RenderTarget* pTarget, int drawTargetNum = -1);
	void PopRenderTarget();
	void ColorMask(bool value);
	const Vector2i& GetViewportSize() const { return viewportSize; }
private:
	Vector2i viewportSize;
	GLStatus m_cache;

	struct RenderTargetStack
	{
		RenderTarget* pRenderTarget;
		int drawTargetNum;
	};


	std::stack<RenderTargetStack> m_pRenderTargetStack;
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
		, m_pTexturePlane(nullptr){};
	~RenderResource() {};
	void Build();

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
	void SetRenderTarget(RenderTarget* pRenderTarget) { m_pRenderTarget = pRenderTarget; }
	RenderTarget* GetRenderTarget() { return m_pRenderTarget; }
	const RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }
	const GLBuffer* GetComputeColorTarget() const { return m_pComputeColorTarget; }
	const GLBuffer* GetComputeDepthTarget() const { return m_pComputeDepthTarget; }
	RenderTarget* GetTmpComputeTarget() { return m_pTmpComputeTarget; }
	RenderTarget* GetTmpPostEffectTarget() { return m_pTmpPostEffectTarget; }
	void UpdateLight();
	void UpdateCamera();
	void UpdatePBR();
	void InitRenderTarget(const Vector2& size);
	void SetTexturePlane(RenderTextureNode* pPlane) { m_pTexturePlane = pPlane; };
	const RenderTextureNode* GetTexturePlane() const { return m_pTexturePlane; }
	RenderTarget* GetPostEffectTarget() { return m_pPostEffectTarget; }
	PBRResource* GetPBR() { return m_pPBR; }
private:
	PBRResource* m_pPBR;
	Unique<GLContext> m_pContext;
	Shared<Camera> m_pCamera;
	Shared<Light> m_pLight;
	GLBuffer* m_pCameraGpu;
	GLBuffer* m_pLightGpu;
	GLBuffer* m_pComputeColorTarget;
	GLBuffer* m_pComputeDepthTarget;
	RenderTarget* m_pRenderTarget;
	RenderTarget* m_pPostEffectTarget;
	ShaderTable m_pShaderTable;
	RenderTextureNode* m_pTexturePlane;
	RenderTarget* m_pTmpComputeTarget; // コンピュートシェーダの描画結果をマージするときに一時的に使うターゲット 
	RenderTarget* m_pTmpPostEffectTarget;
};
}

#endif RENDER_RESOURCE_H
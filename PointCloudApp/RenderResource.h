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

struct Viewport
{
	Viewport(const Vector2i& size) :m_value(Vector4i(0, 0, size.x, size.y)) {}
	Viewport(const Vector4i& value) :m_value(value) {}
	enum Anchor
	{
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

	const Vector4i& Get() const { return m_value; }

	static Viewport Create(const Vector2i& windowSize, const Vector2i& ratioSize, Anchor anghor);
	static std::array<Viewport, 2> SplitHorizontal(const Viewport& viewport);
	static std::array<Viewport, 2> SplitVertical(const Viewport& viewport);
private:
	Vector4i m_value;
};


class GLContext
{
public:
	GLContext() {}
	~GLContext() {};

	void SetupStatus(const GLStatus& status);
	void EnablePolygonOffset(int factor, int units);
	void DisablePolygonOffset();
	void EnableDepth();
	void DisableDepth();
	void EnableCullFace();
	void DisableCullFace();
	void EnablePolygonWire();
	void EnablePolygonFill();
	void EnableScissor(const Viewport& scissor);
	void DisableScissor();


	void SetWindowSize(const Vector2i& size);
	
	
	
	void Clear(GLuint clear);

	Viewport CreateViewport(const Vector2i& ratioSize, Viewport::Anchor anghor) const;
	void SetViewport(const Viewport& viewport);
	void SetViewportFullWindow();
	void SetPointSize(float value);
	void SetLineWidth(float value);

	void SetupPick();
	void SetupShading();

	void PushRenderTarget(RenderTarget* pTarget, int drawTargetNum = -1);
	void PopRenderTarget();
	void ColorMask(bool value);
	const Vector2i& GetWindowSize() const { return m_windowSize; }
private:
	Vector2i m_windowSize = Vector2i(0, 0);
	GLStatus m_cache;

	struct RenderTargetStack
	{
		RenderTarget* pRenderTarget = nullptr;
		int drawTargetNum = 0;
	};


	std::stack<RenderTargetStack> m_pRenderTargetStack;
};

class Camera;
class Light;

namespace ShaderLayout
{
	struct Camera
	{
		Matrix4x4 view;
		Matrix4x4 proj;
		Matrix4x4 vp;
		Matrix4x4 invVP;
		Vector4 eye;
		Vector4 center;
		Vector2 viewSize;
		float padding[2];
		Vector4 frustum[6];
	};

	struct Light
	{
		Vector4 color;
		Vector4 direction;
		float padding[56];
	};
}

class RenderResource
{
public:
	RenderResource()
		: m_pContext(std::make_unique<GLContext>())
		, m_pCameraGpu(nullptr)
		, m_pDebugCameraGpu(nullptr)
		, m_p2DCameraGpu(nullptr)
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
	const GLBuffer* GetDebugCameraBuffer() const { return m_pDebugCameraGpu; }
	const GLBuffer* Get2DCameraBuffer() const { return m_p2DCameraGpu; }
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
	void UpdateDebugCamera(const Camera& camera);
private:
	PBRResource* m_pPBR;
	Unique<GLContext> m_pContext;
	Shared<Camera> m_pCamera;
	Shared<Light> m_pLight;
	GLBuffer* m_pDebugCameraGpu; 
	GLBuffer* m_pCameraGpu;
	GLBuffer* m_p2DCameraGpu;
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
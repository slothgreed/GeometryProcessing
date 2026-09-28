#ifndef RENDER_RESOURCE_H
#define RENDER_RESOURCE_H
#include "Renderer/Shader/ShaderTable.h"
#include "GL/Buffer/Texture.h"
#include "GL/RenderTarget.h"
#include "Renderer/PBR.h"
#include "GL/GLContext.h"
namespace KI
{

class Camera;
class Light;
class BDB;
class ClusteredLightResource;
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
		float _near;
		float _far;
		Vector4 frustum[6];
	};

	struct Light
	{
		Vector4 color;
		Vector4 direction;
		float padding[56];
	};

	struct PointLight
	{
		Vector4 positionRadius;
		Vector4 colorIntensity;
		Vector4 velocity;
	};
	static_assert(sizeof(PointLight) == sizeof(float) * 12);
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
		, m_pTexturePlane(nullptr)
		{};
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
	const GLBuffer* GetComputeAccumTarget() const { return m_pComputeAccumTarget; }
	RenderTarget* GetTmpComputeTarget() { return m_pTmpComputeTarget; }
	RenderTarget* GetTmpPostEffectTarget() { return m_pTmpPostEffectTarget; }
	void UpdateLight(const Vector2i& windowSize);
	void BuildPointLights(const BDB& bdb, int resolution);
	void BuildPointLights(const BDB& bdb, const Vector3i& resolution);
	void UpdateCamera();
	void UpdatePBR();
	void InitRenderTarget(const Vector2& size);
	const TexturePlane* GetTexturePlane() const { return m_pTexturePlane; }
	RenderTarget* GetPostEffectTarget() { return m_pPostEffectTarget; }
	RenderTarget* GetDebugTarget() { return m_pDebugTarget; }
	PBRResource* GetPBR() { return m_pPBR; }
	void UpdateCamera(const Camera& pCamera);
	void UpdateCamera(const BDB& bdb);
	void UpdateDebugCamera(const Camera& camera);
	void SetTimeDelta(float value) { m_timeDelta = value; }
	float GetTimeDelta() const { return m_timeDelta; }
	ClusteredLightResource* GetClusteredLightResource() { return m_pClusteredLightResource; }
	const ClusteredLightResource* GetClusteredLightResource() const { return m_pClusteredLightResource; }
private:
	float m_timeDelta = 0.0f;
	PBRResource* m_pPBR;
	ClusteredLightResource* m_pClusteredLightResource = nullptr;
	Unique<GLContext> m_pContext;
	Shared<Camera> m_pCamera;
	Shared<Light> m_pLight;
	GLBuffer* m_pDebugCameraGpu; 
	GLBuffer* m_pCameraGpu;
	GLBuffer* m_p2DCameraGpu;
	GLBuffer* m_pLightGpu;
	GLBuffer* m_pComputeColorTarget;
	GLBuffer* m_pComputeDepthTarget;
	GLBuffer* m_pComputeAccumTarget;
	RenderTarget* m_pRenderTarget;
	RenderTarget* m_pPostEffectTarget;
	ShaderTable m_pShaderTable;
	TexturePlane* m_pTexturePlane;
	RenderTarget* m_pTmpComputeTarget; // コンピュートシェーダの描画結果をマージするときに一時的に使うターゲット 
	RenderTarget* m_pTmpPostEffectTarget;
	RenderTarget* m_pDebugTarget;
};
}

#endif RENDER_RESOURCE_H

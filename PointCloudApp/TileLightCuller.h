#ifndef TILE_LIGHT_CULLER_H
#define	TILE_LIGHT_CULLER_H
#include "IShader.h"
#include "BDB.h"
namespace KI
{

struct DrawContext;
class TileLightCuller
{
public:
	TileLightCuller() {};
	~TileLightCuller()
	{
		RELEASE_INSTANCE(m_pUpdateShader);
		RELEASE_INSTANCE(m_pDebugShader);
		RELEASE_INSTANCE(m_pShader);
	};
	
	void Execute(const DrawContext& context);
	void DrawDebugView(const DrawContext& context);
	void Update(const DrawContext& context, const BDB& bdb);

	struct DebugViewShader : IPostEffectShader
	{
		virtual void FetchUniformLocation();
		virtual ShaderPath GetShaderPath();

		void BindTileCount(const Vector2i& tileCount);
		void BindTileSize(const Vector2i& tileSize);
		void BindMaxLightNum();
		GLuint m_uMaxLightNum = -1;
		GLuint m_uTileCount = -1;
		GLuint m_uTileSize = -1;
	};

	struct UpdateShader : public IComputeShader
	{
		virtual void FetchUniformLocation();
		virtual ShaderPath GetShaderPath();
		void BindBoundingBox(const Vector3& min, const Vector3& max);
		void BindTimeDelta(float timeDelta);

		GLuint m_uBDBMin = -1;
		GLuint m_uBDBMax = -1;
		GLuint m_uTimeDelta = -1;
	};

	struct Shader : public IComputeShader
	{
		virtual void FetchUniformLocation();
		virtual ShaderPath GetShaderPath();
		virtual Vector3i GetLocalThreadNum() const { return Vector3i(PIXEL_SIZE, PIXEL_SIZE, 1); }
		Vector2i GetLocalThread2D() const { return Vector2i(PIXEL_SIZE, PIXEL_SIZE); }

		void BindDepth(const Texture* pTexture);
		void BindWindowSize(const Vector2i& windowSize);
		void BindLocalSize(const Vector2i& localSize);
		void BindLightNum(int lightNum);

		GLuint m_uDepth = -1;
		GLuint m_uWindowSize = -1;
		GLuint m_uLocalSize = -1;
		GLuint m_uLightNum = -1;
	};

	Vector2i GetTileSize() const { return Vector2i(PIXEL_SIZE, PIXEL_SIZE); }
	static Vector2i GetTileCount2D(const Vector2i& windowSize);
	static int GetTileCount1D(const Vector2i& windowSize);

private:
	static constexpr int PIXEL_SIZE = 16;
	static constexpr int MAX_LIGHT_NUM = 8;
	void BuildResource(const Vector2i& windowSize);
	Shader* m_pShader = nullptr;
	DebugViewShader* m_pDebugShader = nullptr;
	UpdateShader* m_pUpdateShader = nullptr;

};

class TileLightResource
{
public:
	TileLightResource() {}
	~TileLightResource()
	{
		RELEASE_INSTANCE(m_pPointLightGpu);
		RELEASE_INSTANCE(m_pTileLight);
	}

	const GLBuffer* GetTileLightBuffer() const { return m_pTileLight; }
	const GLBuffer* GetPointLightBuffer() const { return m_pPointLightGpu; }
	void BuildPointLights(const BDB& bdb, int resolution);
	void BuildPointLights(const BDB& bdb, const Vector3i& resolution);
	void BuildPointLightBuffer(const Vector2i& windowSize);
private:
	BDB m_bdb;
	GLBuffer* m_pTileLight = nullptr;
	GLBuffer* m_pPointLightGpu = nullptr;
};


}


#endif
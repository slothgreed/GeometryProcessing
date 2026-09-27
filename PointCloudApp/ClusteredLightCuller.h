#ifndef CLUSTERED_LIGHT_CULLER_H
#define	CLUSTERED_LIGHT_CULLER_H
#include "IShader.h"
#include "BDB.h"
namespace KI
{

struct DrawContext;
class ClusteredLightResource;
class ClusteredLightCuller
{
public:
	static constexpr int MAX_LIGHT_NUM = 64;
	ClusteredLightCuller() {};
	~ClusteredLightCuller()
	{
		RELEASE_INSTANCE(m_pUpdateShader);
		RELEASE_INSTANCE(m_pDebugShader);
		RELEASE_INSTANCE(m_pShader);
	};
	
	void ShowUI(ClusteredLightResource& resource, const BDB& bdb, const Vector2i& windowSize);
	bool IsEnabled() const { return m_ui.enabled; }
	bool IsAnimationEnabled() const { return m_ui.animation; }
	int GetDebugMode() const { return m_ui.debugMode; }

	void Execute(const DrawContext& context);
	void DrawDebugView(const DrawContext& context, int displayMode);
	void Update(const DrawContext& context, const BDB& bdb);

	struct DebugViewShader : IPostEffectShader
	{
		virtual void FetchUniformLocation();
		virtual ShaderPath GetShaderPath();

		void BindClusterCount(const Vector2i& clusterCount);
		void BindClusterPartition(const Vector3i& clusterPartition);
		void BindMaxLightNum();
		void BindDisplayMode(int displayMode);
		void BindSlice(int slice);
		void BindShowGrid(bool showGrid);
		GLuint m_uSlice = -1;
		GLuint m_uShowGrid = -1;
		GLuint m_uDisplayMode = -1;
		GLuint m_uMaxLightNum = -1;
		GLuint m_uClusterCount = -1;
		GLuint m_uClusterPartition = -1;
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
		void BindLightNum(int lightNum);
		void BindClusterPartition(const Vector3i& clusterNum);

		GLuint m_uDepth = -1;
		GLuint m_uWindowSize = -1;
		GLuint m_uClusterPartition = -1;
		GLuint m_uLightNum = -1;
	};

	static Vector3i GetClusterPartition() { return Vector3i(PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE); }
	static Vector3i GetClusterCount3D(const Vector2i& windowSize);
	static int GetClusterCount1D(const Vector2i& windowSize);

private:
	struct UI
	{
		bool enabled = false;
		bool animation = false;
		// Hidden, minimum depth, maximum depth, light count.
		int debugMode = 0;
		int debugSlice = 0;
		bool debugGrid = true;
		int pointLightCountIndex = 0; // One light at the bounding box center.
	};
	UI m_ui;
	static constexpr int PIXEL_SIZE = 16;
	void BuildResource(const Vector2i& windowSize);
	Shader* m_pShader = nullptr;
	DebugViewShader* m_pDebugShader = nullptr;
	UpdateShader* m_pUpdateShader = nullptr;

};

class ClusteredLightResource
{
public:
	ClusteredLightResource() {}
	~ClusteredLightResource()
	{
		RELEASE_INSTANCE(m_pPointLightGpu);
		RELEASE_INSTANCE(m_pClusteredLight);
	}

	bool IsActive() const { return m_pClusteredLight != nullptr && m_pPointLightGpu != nullptr; }
	const GLBuffer* GetClusteredLightBuffer() const { return m_pClusteredLight; }
	const GLBuffer* GetPointLightBuffer() const { return m_pPointLightGpu; }
	const BDB& GetBoundingBox() const { return m_bdb; }
	void BuildPointLights(const BDB& bdb, int resolution);
	void BuildPointLights(const BDB& bdb, const Vector3i& resolution);
	void BuildPointLightBuffer(const Vector2i& windowSize);
private:
	BDB m_bdb;
	GLBuffer* m_pClusteredLight = nullptr;
	GLBuffer* m_pPointLightGpu = nullptr;
};


}


#endif

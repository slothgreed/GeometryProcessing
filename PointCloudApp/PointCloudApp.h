#ifndef POIINT_CLOUD_APP_H
#define	POIINT_CLOUD_APP_H

#include "RenderResource.h"
#include "RenderNode.h"
#include "GLFWApp.h"
#include "Profiler.h"
namespace KI
{
class CSFRenderNode;
class HalfEdgeStruct;
class HalfEdgeNode;
class PointCloudNode;
class InstancedPrimitiveNode;
class PointCloudApp : public GLFWApp
{
public:
	PointCloudApp()
		: m_pSelect(nullptr)
		, m_gpuProfiler(nullptr)
	{
	};
	~PointCloudApp() {};

	void Execute();
	void Finalize();

	virtual void ProcessMouseEvent(const MouseInput& input);
	virtual void ResizeEvent(int width, int height);
private:

	struct UI
	{
		UI()
		:pickMode(false)
		,animation(false)
		,visibleSkyBox(true)
		,visibleTexture(false)
		,mipmap(0){}
		~UI() {}
		bool pickMode;
		bool visibleSkyBox;
		bool visibleTexture;
		int mipmap;
		bool animation;
	};

	PickResult m_pick;
	UI m_ui;
	UIContext m_uiContext;
	Shared<RenderNode> CreateSpaceTest();
	Shared<RenderNode> CreateGLTFAnimationTest();
	Shared<RenderNode> CreateGLTFNodeTest();
	Shared<RenderNode> CreateCSFNodeTest();
	Shared<RenderNode> CreatePBRTest();
	Shared<RenderNode> CreateVolumeTest();
	Shared<RenderNode> CreateTerrain();
	Shared<RenderNode> CreateLargePointCloudNodeTest();
	Vector<Shared<RenderNode>> CreateSTEPNodeTest();
	Shared<HalfEdgeNode> CreateBunnyNodeTest();
	Shared<HalfEdgeNode> CreateBunnyNodeTest(const Vector3& pos);
	Shared<PointCloudNode> CreateDelaunayTest();
	Shared<RenderNode> CreateConstrainDelaunayTest();
	Shared<InstancedPrimitiveNode> CreateInstacedNodeTest();
	Shared<RenderResource> m_pResource;
	CPUProfiler m_cpuProfiler;
	GPUProfiler* m_gpuProfiler;
	Unique<RenderNode> m_pRoot;
	RenderNode* m_pSelect;
	void ShowUI(UIContext& ui);
	Shared<RenderNode> m_pGLTFAnimation;
	Shared<HalfEdgeStruct> m_pBunny;
};
}

#endif // POIINT_CLOUD_APP_H
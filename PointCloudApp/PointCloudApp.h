#ifndef POIINT_CLOUD_APP_H
#define	POIINT_CLOUD_APP_H

#include "RenderResource.h"
#include "RenderNode.h"
#include "GLFWApp.h"
#include "Profiler.h"
namespace KI
{
class CSFRenderNode;
class HalfEdgeNode;
class PointCloudNode;
class InstancedPrimitiveNode;
class PointCloudApp : public GLFWApp
{
public:
	PointCloudApp()
		: m_pSelect(nullptr)
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
		,animation(false){}
		~UI() {}
		bool pickMode;
		bool animation;
	};

	PickResult m_pick;
	UI m_ui;
	Shared<RenderNode> CreateGLTFNodeTest();
	Shared<RenderNode> CreateCSFNodeTest();
	Shared<RenderNode> CreateLargePointCloudNodeTest();
	Shared<RenderNode> CreateSTEPNodeTest();
	Shared<HalfEdgeNode> CreateBunnyNodeTest();
	Shared<PointCloudNode> CreateDelaunayTest();
	Shared<InstancedPrimitiveNode> CreateInstacedNodeTest();
	void ShowUI();
	Shared<RenderResource> m_pResource;
	CPUProfiler m_cpuProfiler;
	Unique<RenderNode> m_pRoot;
	RenderNode* m_pSelect;
};
}

#endif // POIINT_CLOUD_APP_H
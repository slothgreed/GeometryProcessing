#ifndef POIINT_CLOUD_APP_H
#define	POIINT_CLOUD_APP_H

#include "RenderResource.h"
#include "RenderNode.h"
#include "GLFWApp.h"
#include "Profiler.h"
namespace KI
{
class InstancedPrimitiveNode;
class PointCloudApp : public GLFWApp
{
public:
	PointCloudApp() {};
	~PointCloudApp() {};

	void Execute();
	void Finalize();

	virtual void ProcessMouseEvent(const MouseInput& input);
	virtual void ResizeEvent(int width, int height);
private:
	Shared<InstancedPrimitiveNode> CreateInstacedNodeTest();
	void ShowUI();
	CPUProfiler m_cpuProfiler;
	Unique<RenderNode> m_pRoot;
};
}

#endif // POIINT_CLOUD_APP_H
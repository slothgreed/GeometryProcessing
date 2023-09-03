#ifndef DRAE_CALL_TEST_H
#define	DRAE_CALL_TEST_H

#include "CameraController.h"
#include "RenderResource.h"
#include "RenderNode.h"
class PointCloudApp
{
public:
	PointCloudApp() {};
	~PointCloudApp() {};

	void Execute();
	void Finalize();
	void ProcessMouseEvent(const MouseInput& input);
	void ResizeEvent(int width, int height);

	RenderResource* GetResource() { return m_pResource.get(); }
	static PointCloudApp* Application();
private:
	std::unique_ptr<RenderNode> m_pRoot;
	std::unique_ptr<Mouse> m_pMouse;
	std::shared_ptr<Camera> m_pCamera;
	std::unique_ptr<CameraController> m_pCameraController;
	std::unique_ptr<RenderResource> m_pResource;
};


#endif // DRAE_CALL_TEST_H
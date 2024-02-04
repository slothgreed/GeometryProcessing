#ifndef GRAPHICS_APP_H
#define	GRAPHICS_APP_H
#include "TheApp.h"
#include "RenderResource.h"
#include "CameraController.h"
class GLFWApp : public TheApp
{
public:
	GLFWApp() {};
	~GLFWApp() {};

	virtual void Initialize();
	virtual void Execute();
	virtual void Finalize();

	virtual void ProcessMouseEvent(const MouseInput& input) {};
	virtual void ResizeEvent(int width, int height) {};
	static GLFWApp* Application();
	RenderResource* GetResource() { return m_pResource.get(); }
protected:
	GLFWwindow* m_window;
	std::unique_ptr<Mouse> m_pMouse;
	std::shared_ptr<Camera> m_pCamera;
	std::unique_ptr<CameraController> m_pCameraController;
	std::unique_ptr<RenderResource> m_pResource;
};

#endif // GRAPHICS_APP_H
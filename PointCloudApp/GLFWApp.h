#ifndef GRAPHICS_APP_H
#define	GRAPHICS_APP_H
#include "TheApp.h"
#include "RenderResource.h"
#include "CameraController.h"
namespace KI
{

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
protected:
	Vector2 m_windowSize;
	GLFWwindow* m_window;
	Unique<Mouse> m_pMouse;
	Shared<Camera> m_pCamera;
	Unique<CameraController> m_pCameraController;
};
}

#endif // GRAPHICS_APP_H
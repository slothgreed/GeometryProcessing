#ifndef GRAPHICS_APP_H
#define	GRAPHICS_APP_H
#include "App/TheApp.h"
#include "GL/RenderResource.h"
#include "Renderer/Camera/CameraController.h"
#include "Utility/MouseInput.h"
namespace KI
{

class GLFWApp : public TheApp
{
public:
	GLFWApp()
		: m_window(nullptr)
		, m_vertexArrayId(0)
	{};
	~GLFWApp() {};

	virtual void Initialize();
	virtual void Execute();
	virtual void Finalize();

	virtual void ProcessMouseEvent(const MouseInput& input) {};
	virtual void ResizeEvent(int width, int height) {};
	static GLFWApp* Application();
protected:
	Vector2i m_windowSize;
	GLFWwindow* m_window;
	Unique<Mouse> m_pMouse;
	Shared<Camera> m_pCamera;
	Unique<CameraController> m_pCameraController;
	GLuint m_vertexArrayId;
};
}

#endif // GRAPHICS_APP_H

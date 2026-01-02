#include <iostream>
#include "GLFWApp.h"
#include "MouseInput.h"
#include "GLAPIExt.h"
namespace KI
{

GLFWApp* g_instance;

GLFWApp* GLFWApp::Application()
{
	return g_instance;
}
void ScrollCallBack(GLFWwindow* window, double x, double y)
{
	MouseInput input;
	input.SetWheel((int)y);
	input.SetEvent(MY_MOUSE_EVENT::MOUSE_EVENT_WHEEL);
	g_instance->ProcessMouseEvent(input);
}

void WindowSizeCallBack(GLFWwindow* window, int width, int height)
{

	g_instance->ResizeEvent(width, height);
}

void CursorPosCallBack(GLFWwindow* window, double xpos, double ypos)
{
	MouseInput input;
	input.SetPosition((float)xpos, (float)ypos);
	input.SetEvent(MY_MOUSE_EVENT::MOUSE_EVENT_MOVE);

	if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
		input.SetPress(MY_MOUSE_BUTTON::MOUSE_BUTTON_LEFT);
	} else {
		input.SetRelease(MY_MOUSE_BUTTON::MOUSE_BUTTON_LEFT);
	}

	if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE)) {
		input.SetPress(MY_MOUSE_BUTTON::MOUSE_BUTTON_MIDDLE);
	} else {
		input.SetRelease(MY_MOUSE_BUTTON::MOUSE_BUTTON_MIDDLE);
	}

	if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {
		input.SetPress(MY_MOUSE_BUTTON::MOUSE_BUTTON_RIGHT);
	} else {
		input.SetRelease(MY_MOUSE_BUTTON::MOUSE_BUTTON_RIGHT);
	}

	g_instance->ProcessMouseEvent(input);
}

void MouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	MY_MOUSE_BUTTON mouseButton;
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		mouseButton = MOUSE_BUTTON_LEFT;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		mouseButton = MOUSE_BUTTON_MIDDLE;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		mouseButton = MOUSE_BUTTON_RIGHT;
		break;
	default:
		break;
	}

	double posX;
	double posY;
	glfwGetCursorPos(window, &posX, &posY);

	MouseInput input;
	input.SetPosition((float)posX, (float)posY);
	if (action == GLFW_PRESS) {
		input.SetEvent(MOUSE_EVENT_DOWN);
		input.SetPress(mouseButton);
	} else {
		input.SetEvent(MOUSE_EVENT_UP);
		input.SetRelease(mouseButton);
	}

	g_instance->ProcessMouseEvent(input);
}


void GLFWApp::Initialize()
{
	if (glfwInit() == GL_FALSE) {
		std::cerr << "Can't initilize GLFW" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	m_windowSize = Vector2i(1024, 768);
	m_window = glfwCreateWindow(m_windowSize.x, m_windowSize.y, "PointCloudApp", NULL, NULL);
	if (m_window == NULL) {
		return;
	}

	glfwMakeContextCurrent(m_window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		return;
	}

	if (!GLAPIExt::Initialize()) {
		return;
	}

	// OpenGLバージョンとGLSLバージョンの確認
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "Driver version: " << glGetString(GL_RENDERER) << std::endl;
	g_instance = this;
	glfwSetCursorPosCallback(m_window, CursorPosCallBack);
	glfwSetMouseButtonCallback(m_window, MouseButtonCallBack);
	glfwSetScrollCallback(m_window, ScrollCallBack);
	glfwSetWindowSizeCallback(m_window, WindowSizeCallBack);
	glfwSwapInterval(0);

	m_pMouse = std::make_unique<Mouse>();
	m_pCamera = std::make_shared<Camera>();
	m_pCamera->SetPerspective(45, 1, 0.1, 1000000);
	//m_pCamera->SetOrtho(Camera::Ortho(-300, 300, 300, -300, -300, 300));
	m_pCamera->SetLookAt(Vector3(0, 0, -1), Vector3(0, 0, 0), Vector3(0, 1, 0));
	m_pCamera->SetViewport(Vector4i(0, 0, m_windowSize.x, m_windowSize.y));
	m_pCameraController = std::make_unique<CameraController>(m_pCamera);
}

void GLFWApp::Execute()
{

}

void GLFWApp::Finalize()
{
	GLAPIExt::Finalize();
	g_instance = NULL;
	glfwTerminate();
}
}

// PointCloudApp.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//


#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Mouse.h"
#include "MouseInput.h"
#include "PointCloudApp.h"
#include "CameraController.h"
#include "Camera.h"
#include "Profiler.h"
#include "PointCloud.h"
#include "PointCloudNode.h"
#include "Primitive.h"
#include "KMeansAlgorithm.h"
#include "Utility.h"
#include "Primitives.h"
#include "PrimitiveNode.h"
#include <Eigen/Core>

PointCloudApp* m_instance;
PointCloudApp* PointCloudApp::Application()
{
	return m_instance;
}
void ScrollCallBack(GLFWwindow* window, double x, double y)
{
	MouseInput input;
	input.SetWheel((int)y);
	input.SetEvent(MY_MOUSE_EVENT::MOUSE_EVENT_WHEEL);
	PointCloudApp::Application()->ProcessMouseEvent(input);
}

void WindowSizeCallBack(GLFWwindow* window, int width, int height)
{
	PointCloudApp::Application()->ResizeEvent(width, height);
}

void CursorPosCallBack(GLFWwindow* window, double xpos, double ypos)
{
	MouseInput input;
	input.SetPosition((float)xpos, (float)ypos);
	input.SetEvent(MY_MOUSE_EVENT::MOUSE_EVENT_MOVE);

	if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
	{
		input.SetPress(MY_MOUSE_BUTTON::MOUSE_BUTTON_LEFT);
	}
	else
	{
		input.SetRelease(MY_MOUSE_BUTTON::MOUSE_BUTTON_LEFT);
	}

	if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE))
	{
		input.SetPress(MY_MOUSE_BUTTON::MOUSE_BUTTON_MIDDLE);
	}
	else
	{
		input.SetRelease(MY_MOUSE_BUTTON::MOUSE_BUTTON_MIDDLE);
	}

	if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
	{
		input.SetPress(MY_MOUSE_BUTTON::MOUSE_BUTTON_RIGHT);
	}
	else
	{
		input.SetRelease(MY_MOUSE_BUTTON::MOUSE_BUTTON_RIGHT);
	}

	PointCloudApp::Application()->ProcessMouseEvent(input);
}

void MouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	MY_MOUSE_BUTTON mouseButton;
	switch (button)
	{
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
	if (action == GLFW_PRESS)
	{
		input.SetEvent(MOUSE_EVENT_DOWN);
		input.SetPress(mouseButton);
	}
	else
	{
		input.SetEvent(MOUSE_EVENT_UP);
		input.SetRelease(mouseButton);
	}

	PointCloudApp::Application()->ProcessMouseEvent(input);
}

void PointCloudApp::ResizeEvent(int width, int height)
{
	glViewport(0, 0, width, height);
	if (m_pCamera) {
		m_pCamera->SetAspect(width / (float)height);
	}
}
void PointCloudApp::ProcessMouseEvent(const MouseInput& input)
{
	m_pMouse->ApplyMouseInput(input);

	if (input.Event() == MOUSE_EVENT_WHEEL)
	{
		m_pCameraController->Wheel(*m_pMouse.get());
	}
	else if (input.Event() == MOUSE_EVENT_MOVE)
	{
		m_pCameraController->Move(*m_pMouse.get());
	}
}

void PointCloudApp::Execute()
{
	if (glfwInit() == GL_FALSE)
	{
		std::cerr << "Can't initilize GLFW" << std::endl;
		return;
	}

	GLFWwindow* window = glfwCreateWindow(1024, 768, "PointCloudApp", NULL, NULL);
	if (window == NULL) {
		return;
	}
	glfwMakeContextCurrent(window); 
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		return;
	}

	Eigen::Matrix3d mat;
	mat.Random();


	m_instance = this;
	m_pMouse = std::make_unique<Mouse>();
	m_pCamera = std::make_shared<Camera>();
	m_pResource = std::make_unique<RenderResource>();
	m_pResource->Build();

	m_pCamera->SetPerspective(45, 1, 0.1, 10000);
	m_pCamera->SetLookAt(vec3(0, 0, -1), vec3(0, 0, 0), vec3(0, 1, 0));
	m_pCameraController = std::make_unique<CameraController>(m_pCamera);
	glfwSetCursorPosCallback(window, CursorPosCallBack);
	glfwSetMouseButtonCallback(window, MouseButtonCallBack);
	glfwSetScrollCallback(window, ScrollCallBack);	
	glfwSetWindowSizeCallback(window, WindowSizeCallBack);
	glfwSwapInterval(0);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	//primitives.push_back(std::shared_ptr<PointCloud>(PointCloud::Load("E:\\cgModel\\pointCloud\\pcd\\rops_cloud.pcd")));
	//auto pPointCloud = (std::shared_ptr<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\dragon.xyz")));
	//auto pPointCloud = (std::shared_ptr<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\cube.xyz")));
	//auto pPointCloud = (std::shared_ptr<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\bunny4000.xyz")));
	auto pPointCloud = (std::shared_ptr<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\Armadillo.xyz")));
	//auto pPointCloud = (std::shared_ptr<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\lucy.xyz")));


	//auto pPointCloud = std::shared_ptr<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\random_10.xyz"));
	
	//auto pRandom = std::shared_ptr<PointCloud>(PointCloud::Create2D(10, vec2(0, 0), vec2(100, 100)));
	//pRandom->OutputText("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\random_10.xyz");


	std::vector<vec3> color(pPointCloud->Position().size(), vec3(1.0f, 1.0f, 1.0f));
	pPointCloud->SetColor(std::move(color));
	//KMeansAlgorithm kmeans(pPointCloud, 300, 10);
	//kmeans.Execute();
	//pPointCloud->SetColor(kmeans.CreateClusterColor());
	BDB bdb;
	bdb.Apply(pPointCloud->GetBDB());

	m_pRoot = std::make_unique<RenderNode>("Root");

	//m_algorithm[ALGORITHM_KDTREE] = new KDTree(pNode, 2);

	std::shared_ptr<Primitive> pAxis = std::make_shared<Axis>();
	m_pRoot->SetNode(std::make_shared<PointCloudNode>("PointCloud", pPointCloud));
	m_pRoot->SetNode(std::make_shared<PrimitiveNode>("Axis", pAxis));
	m_pCamera->FitToBDB(bdb);
	
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);	// GLenum mode
	glPointSize(5.0f);
	Profile profiler;

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 400 core");

	//GLuint VertexArrayID;
	//glGenVertexArrays(1, &VertexArrayID);
	//glBindVertexArray(VertexArrayID);
	while (glfwWindowShouldClose(window) == GL_FALSE)
	{
		profiler.Start();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_pRoot->Draw(m_pCamera->Projection(), m_pCamera->ViewMatrix());

		profiler.Stop();
		//profiler.Output();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		m_pRoot->ShowUIData();

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}


}

void PointCloudApp::Finalize()
{
	m_pRoot.reset();
	glfwTerminate();
}

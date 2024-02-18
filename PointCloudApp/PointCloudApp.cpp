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
#include "PointCloudIO.h"
#include "PointCloud.h"
#include "PointCloudNode.h"
#include "Primitive.h"
#include "KMeansAlgorithm.h"
#include "Utility.h"
#include "Primitives.h"
#include "PrimitiveNode.h"
#include "TextureLoader.h"
#include "RenderTextureNode.h"
#include "GLTFLoader.h"
#include "GLUtility.h"
#include <Eigen/Core>
namespace KI
{
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
	Eigen::Matrix3d mat;
	mat.Random();


	m_pResource = std::make_unique<RenderResource>();
	m_pResource->Build();

	m_pRoot = std::make_unique<RenderNode>("Root");

	auto pGLTF = GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\Sponza\\glTF\\Sponza.gltf");
	//auto pGLTF = GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\2CylinderEngine\\glTF\\2CylinderEngine.gltf");
	m_pRoot->AddNode(std::shared_ptr<RenderNode>(pGLTF));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\cgModel\\pointCloud\\pcd\\rops_cloud.pcd")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\dragon.xyz")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\cube.xyz")));
	auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\bunny4000.xyz")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\Armadillo.xyz")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\cgModel\\pointCloud\\bildstein_station3_xyz_intensity_rgb.xyz")));
	
	//pPointCloud->Multi(glm::rotate(-90.0f, Vector3(1, 0, 0)));
	//pPointCloud->To2D();
	//auto pPointCloud = (Shared<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\lucy.xyz")));


	//auto pPointCloud = Shared<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\random_10.xyz"));
	
	//auto pRandom = Shared<PointCloud>(PointCloud::Create2D(10, vec2(0, 0), vec2(100, 100)));
	//pRandom->OutputText("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\random_10.xyz");


	Vector<Vector3> color(pPointCloud->Position().size(), Vector3(1.0f, 1.0f, 1.0f));
	pPointCloud->SetColor(std::move(color));
	//KMeansAlgorithm kmeans(pPointCloud, 300, 10);
	//kmeans.Execute();
	//pPointCloud->SetColor(kmeans.CreateClusterColor());
	BDB bdb;
	bdb.Apply(pPointCloud->GetBDB());


	Shared<Primitive> pAxis = std::make_shared<Axis>();
	m_pRoot->AddNode(std::make_shared<PointCloudNode>("PointCloud", pPointCloud));
	m_pRoot->AddNode(std::make_shared<PrimitiveNode>("Axis", pAxis));
	m_pCamera->FitToBDB(bdb);
	
	
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);	// GLenum mode
	//glPointSize(5.0f);
	glLineWidth(5.0f);
	CPUProfiler cpuProfiler;

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 400 core");

	GPUProfiler render = GPUProfiler("Render");

	//GLuint VertexArrayID;
	//glGenVertexArrays(1, &VertexArrayID);
	//glBindVertexArray(VertexArrayID);
	while (glfwWindowShouldClose(m_window) == GL_FALSE)
	{
		cpuProfiler.Start();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render.Start();
		m_pRoot->Draw(m_pCamera->Projection(), m_pCamera->ViewMatrix());
		render.Stop();

		cpuProfiler.Stop();
		cpuProfiler.Output();

		//ImGui_ImplOpenGL3_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();

		//m_pRoot->ShowUIData();

		//ImGui::Render();
		//int display_w, display_h;
		//glfwGetFramebufferSize(m_window, &display_w, &display_h);
		//glViewport(0, 0, display_w, display_h);
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}


}

void PointCloudApp::Finalize()
{
	m_pRoot.reset();
	glfwTerminate();
}
}
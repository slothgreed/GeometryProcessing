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
#include "HalfEdgeLoader.h"
#include "HalfEdgeNode.h"
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

// utility structure for realtime plot
struct RollingBuffer
{
	float Span;
	ImVector<ImVec2> Data;
	RollingBuffer()
	{
		Span = 10.0f;
		Data.reserve(2000);
	}
	void AddPoint(float x, float y)
	{
		float xmod = fmodf(x, Span);
		if (!Data.empty() && xmod < Data.back().x)
			Data.shrink(0);
		Data.push_back(ImVec2(xmod, y));
	}
};

struct ScrollingBuffer
{
	int MaxSize;
	int Offset;
	ImVector<ImVec2> Data;
	ScrollingBuffer()
	{
		MaxSize = 2000;
		Offset = 0;
		Data.reserve(MaxSize);
	}
	void AddPoint(float x, float y)
	{
		if (Data.size() < MaxSize)
			Data.push_back(ImVec2(x, y));
		else {
			Data[Offset] = ImVec2(x, y);
			Offset = (Offset + 1) % MaxSize;
		}
	}
	void Erase()
	{
		if (Data.size() > 0) {
			Data.shrink(0);
			Offset = 0;
		}
	}
};

void PointCloudApp::Execute()
{
	auto pResource = std::make_unique<RenderResource>();
	pResource->Build();

	BDB bdb;
	m_pRoot = std::make_unique<RenderNode>("Root");
	m_pRoot = (std::unique_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\Sponza\\glTF\\Sponza.gltf")));
	//m_pRoot->SetMatrix(glmUtil::CreateRotate(-glm::pi<float>() / 2, Vector3(0, 1, 0)));
	bdb.Add(m_pRoot->GetBoundBox());
	
	m_pCamera->SetLookAt(Vector3(0, 0, 1), Vector3(0, 0, 0), m_pCamera->Up());
	//m_pRoot = (std::unique_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\2CylinderEngine\\glTF\\2CylinderEngine.gltf")));

	//m_pRoot->AddNode(std::shared_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\Sponza\\glTF\\Sponza.gltf")));
	//m_pRoot->AddNode(std::shared_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\BoxAnimated\\glTF\\BoxAnimated.gltf")));
	//m_pRoot->AddNode(std::shared_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\RiggedSimple\\glTF\\RiggedSimple.gltf")));
	//m_pRoot->AddNode(std::shared_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\RiggedFigure\\glTF\\RiggedFigure.gltf")));
	//m_pRoot->AddNode(std::shared_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\CesiumMan\\glTF\\CesiumMan.gltf")));
	//m_pRoot->AddNode(std::shared_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\BrainStem\\glTF\\BrainStem.gltf")));

	//auto pGLTF = GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\BrainStem\\glTF\\BrainStem.gltf");
	//m_pRoot->AddNode(std::shared_ptr<RenderNode>(pGLTF));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\cgModel\\pointCloud\\pcd\\rops_cloud.pcd")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\dragon.xyz")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\cube.xyz")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\bunny4000.xyz")));
	/*
	auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\Armadillo.xyz")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\cgModel\\pointCloud\\bildstein_station3_xyz_intensity_rgb.xyz")));
	//Vector<Vector3> color(pPointCloud->Position().size(), Vector3(1.0f, 1.0f, 1.0f));
	//pPointCloud->SetColor(std::move(color));
	bdb.Apply(pPointCloud->GetBDB());

	m_pRoot->AddNode(std::make_shared<PointCloudNode>("PointCloud", pPointCloud));
	*/

	//pPointCloud->Multi(glm::rotate(-90.0f, Vector3(1, 0, 0)));
	//pPointCloud->To2D();
	//auto pPointCloud = (Shared<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\lucy.xyz")));


	//auto pPointCloud = Shared<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\random_10.xyz"));
	
	//auto pRandom = Shared<PointCloud>(PointCloud::Create2D(10, vec2(0, 0), vec2(100, 100)));
	//pRandom->OutputText("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\random_10.xyz");


	//KMeansAlgorithm kmeans(pPointCloud, 300, 10);
	//kmeans.Execute(pPointCloud->Position(),300,10);
	//pPointCloud->SetColor(kmeans.CreateClusterColor());
	

	//Shared<Primitive> pAxis = std::make_shared<Axis>(5000);
	//m_pRoot->AddNode(std::make_shared<PrimitiveNode>("Axis", pAxis));

	//{
	//	Shared<Primitive> pCube = std::make_shared<Cube>(vec3(0, 0, 0), vec3(10, 10, 10));
	//	Vector<Vector3> color(pCube->GetTriangleNum());
	//	int index = 0;
	//	while (index < color.size()) {
	//		color[index] = ColorUtility::CreatePrimary(index);
	//		color[index +1] = ColorUtility::CreatePrimary(index);
	//		index += 2;
	//	}
	//	pCube->SetColor(std::move(color));
	//	m_pRoot->AddNode(std::make_shared<PrimitiveNode>("Cube", pCube));
	//}

	{
		String path = "E:\\cgModel\\bunny6000.half";
		auto data = std::shared_ptr<HalfEdgeStruct>(HalfEdgeLoader::Load(path));
		auto node = std::make_shared<HalfEdgeNode>(path, data);
		node->SetMatrix(glmUtil::CreateScale(0.1f));
		m_pRoot->AddNode(node);
	}

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);	// GLenum mode
	//glPointSize(5.0f);
	glLineWidth(5.0f);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 400 core");

	ImPlot::CreateContext();

	GPUProfiler render = GPUProfiler("Render");
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	Timer timer;
	float m_diff = 0;


	DrawContext context(
		m_pCamera.get(),
		pResource->GetCameraBuffer());
	
	context.pShaderTable = pResource->GetShaderTable();

	while (glfwWindowShouldClose(m_window) == GL_FALSE)
	{
		pResource->UpdateCamera(m_pCamera.get());
		m_cpuProfiler.Start();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render.Start();
		timer.Start();
		m_pRoot->Draw(context);
		m_diff += timer.Stop() * 50;
		m_pRoot->Update(m_diff);
		if (m_diff > 100000.0) { m_diff = 0.0f; }
		render.Stop();
		
		m_cpuProfiler.Stop();
		//cpuProfiler.Output();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ShowUI();
		m_pRoot->ShowUIData();

		ImGui::Render();

		int display_w, display_h;
		glfwGetFramebufferSize(m_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(m_window);

		glfwPollEvents();
		OUTPUT_GLERROR;
	}

	ImPlot::DestroyContext();
	ImGui::DestroyContext();

}

void PointCloudApp::ShowUI()
{

	ImGui::Text(
		"Eye:(%lf,%lf,%lf)\nCenter:(%lf,%lf,%lf)\nUp:(%lf,%lf,%lf)\n",
		m_pCamera->Eye().x, m_pCamera->Eye().y, m_pCamera->Eye().z,
		m_pCamera->Center().x, m_pCamera->Center().y, m_pCamera->Center().z,
		m_pCamera->Up().x, m_pCamera->Up().y, m_pCamera->Up().z);

	static RollingBuffer  ui_fpsDraw, ui_fps60, ui_fps120;
	static float timeDelta = 0.0f;
	ui_fpsDraw.Span = 2.0f;
	ui_fps60.Span = 2.0f;
	ui_fps120.Span = 2.0f;
	timeDelta += ImGui::GetIO().DeltaTime;
	ui_fpsDraw.AddPoint(timeDelta, m_cpuProfiler.GetMilli());
	ui_fps60.AddPoint(timeDelta, 1000.0f / 60.0f);
	ui_fps120.AddPoint(timeDelta, 1000.0f / 120.0f);
	ImPlot::SetNextPlotLimitsX(0, 2.0f, ImGuiCond_Always);
	ImPlot::SetNextPlotLimitsY(0, 30, ImGuiCond_Always);
	if (ImPlot::BeginPlot("##Rolling", NULL, NULL, ImVec2(-1, 150), 0, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels)) {
		ImPlot::PlotLine("Draw FPS", &ui_fpsDraw.Data[0].x, &ui_fpsDraw.Data[0].y, ui_fpsDraw.Data.size(), 0, 2 * sizeof(float));
		ImPlot::PlotLine("60 FPS", &ui_fps60.Data[0].x, &ui_fps60.Data[0].y, ui_fps60.Data.size(), 0, 2 * sizeof(float));
		ImPlot::PlotLine("120 FPS", &ui_fps120.Data[0].x, &ui_fps120.Data[0].y, ui_fps120.Data.size(), 0, 2 * sizeof(float));
		ImPlot::EndPlot();
	}

}

void PointCloudApp::Finalize()
{
	m_pRoot.reset();
	glfwTerminate();
}
}
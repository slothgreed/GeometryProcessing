// PointCloudApp.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//


#include <iostream>

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
#include "PostEffect.h"
#include "GLTFLoader.h"
#include "HalfEdgeLoader.h"
#include "HalfEdgeNode.h"
#include "SkyBoxNode.h"
#include "RenderTarget.h"
#include "CSFNode.h"
#include "STEPNode.h"
#include "Utility.h"
#include "SimulationNode.h"
#include <Eigen/Core>
namespace KI
{
void PointCloudApp::ResizeEvent(int width, int height)
{
	m_windowSize = Vector2(width, height);
	if (m_pResource) {
		m_pResource->GL()->SetViewport(Vector2(width, height));
	}
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

// コールバック関数を定義
void APIENTRY MyGLDebugCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam)
{
	fprintf(stderr, "GL DEBUG: %s\n", message);
}

void PointCloudApp::Execute()
{

	m_pResource = std::make_unique<RenderResource>();
	m_pResource->Build();
	m_pRoot = std::make_unique<RenderNode>("Root");
	BDB bdb;
	{
		//m_pRoot->AddNode(CreateSpaceTest());
		//m_pRoot->AddNode(CreateCSFNodeTest());
		//m_pRoot->AddNode(CreateGLTFAnimationTest());
		//m_pRoot->AddNode(CreateGLTFNodeTest());
		m_pRoot->AddNode(CreateBunnyNodeTest());
		bdb.Add(m_pRoot->GetChild().begin()->second->GetBoundBox());
	}

	//m_pRoot->AddNode(CreateCSFNodeTest());
	//m_pRoot->AddNode(CreateGLTFNodeTest());
	//m_pRoot->AddNode(CreateBunnyNodeTest());
	//m_pRoot->AddNode(CreateLargePointCloudNodeTest());

	{
		Shared<Primitive> pAxis = std::make_shared<Axis>(50);
		m_pRoot->AddNode(std::make_shared<PrimitiveNode>("Axis", pAxis));
		//m_pRoot->AddNode(CreateBunnyNodeTest());
		//m_pRoot->AddNode(std::make_shared<SimulationNode>());
		//m_pRoot->AddNode(CreateSTEPNodeTest());
	}

	m_pCamera->SetLookAt(Vector3(0, 0, -1), Vector3(0, 0, 0), m_pCamera->Up());
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\cgModel\\pointCloud\\pcd\\rops_cloud.pcd")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\dragon.xyz")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\cube.xyz")));
	//auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\bunny4000.xyz")));
	//
	
	
	

	//pPointCloud->Multi(glm::rotate(-90.0f, Vector3(1, 0, 0)));
	//pPointCloud->To2D();
	//auto pPointCloud = (Shared<PointCloud>(PointCloud::Load("E:\\MyProgram\\KIProject\\PointCloudApp\\resource\\PointCloud\\lucy.xyz")));



	// Test
	{
		//m_pRoot->AddNode(CreateDelaunayTest());
		//m_pRoot->AddNode(CreateInstacedNodeTest());
	}
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);	// GLenum mode


	// 初期化時に一度だけ設定
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(MyGLDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

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


	auto pSkyBoxNode = std::make_unique<SkyBoxNode>();
	auto pForwardTarget = std::unique_ptr<RenderTarget>(RenderTarget::CreateForwardTarget(m_windowSize));
	Shared<Texture> pMain = pForwardTarget->GetColor(0);
	auto pPickTarget = std::unique_ptr<RenderTarget>(RenderTarget::CreatePickTarget(m_windowSize));
	Shared<Texture> pTexture = pPickTarget->GetColor(0);

	auto pLight = std::make_shared<Light>();
	pLight->SetColor(Vector3(1, 1, 1));
	pLight->SetDirection(Vector3(0, 0, 1));


	auto pTexturePalne = std::make_unique<RenderTextureNode>();
	m_pResource->GL()->SetViewport(m_windowSize);
	m_pResource->GL()->EnablePolygonOffset(1.0f, 1.0f);
	m_pResource->GL()->SetLineWidth(5.0f);
	m_pResource->GL()->SetPointSize(5.0f);
	m_pResource->SetMainCamera(m_pCamera);
	m_pResource->SetLight(pLight);
	m_pResource->SetRenderTarget(pForwardTarget.get());
	m_pResource->SetTexturePlane(pTexturePalne.get());
	DrawContext drawContext(m_pResource.get());
	PickContext pickContext(m_pResource.get());
	ComputeTextureCombiner combiner;
	combiner.Build();
	m_pCameraController->FitToBDB(bdb);

	UIContext ui;
	PostEffect postEffect;
	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		m_pResource->UpdateCamera();
		m_pResource->UpdateLight();
		m_pResource->InitRenderTarget(m_windowSize);
		m_pResource->GL()->PushRenderTarget(pForwardTarget.get(), 1);
		m_pResource->GL()->SetupShading();
		m_cpuProfiler.Start();
		render.Start();
		timer.Start();
		if (m_ui.visibleSkyBox) {
			pSkyBoxNode->Draw(drawContext);
		}
		m_pRoot->Draw(drawContext);


		//combiner.Execute(drawContext);

		m_pResource->GL()->PushRenderTarget(m_pResource->GetPostEffectTarget());
		postEffect.Execute(drawContext);
		m_pResource->GL()->PopRenderTarget();

		m_pResource->GL()->PopRenderTarget();


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, m_windowSize.x, m_windowSize.y);
		//TextureDrawer::Execute(drawContext, m_pResource->GetPostEffectTarget()->GetColor(0).get());
		TextureDrawer::Execute(drawContext, m_pResource->GetRenderTarget()->GetColor(0).get());
		if (m_ui.pickMode) {
			m_pResource->GL()->SetupPick();
			pPickTarget->Resize(m_windowSize);
			m_pResource->GL()->PushRenderTarget(pPickTarget.get());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_pRoot->Pick(pickContext);
			auto mousePos = ImGui::GetMousePos();
			m_pick.context = &pickContext;
			Vector2i screen = Vector2i(mousePos.x, m_windowSize.y - mousePos.y);
			m_pick.id = pPickTarget->GetIntPixel(screen.x, screen.y);
			auto depth = pPickTarget->GetDepth(screen.x, screen.y);
			m_pick.pickPos = m_pCamera->ScreenToModel(Vector3(screen.x, screen.y, depth));
			pickContext.pickedId = m_pick.id;
			m_pRoot->CollectPicked(m_pick);
			m_pResource->GL()->PopRenderTarget();
			if (m_pick.pResult.size() != 0) {
				for (auto& result : m_pick.pResult) {
					result.first->DrawParts(drawContext, *result.second.get());
				}
			}

			glViewport(0, 0, 256, 256);
			TextureDrawer::Execute(drawContext, pTexture.get());
		}

		glViewport(0, 0, 256, 256);
		TextureDrawer::Execute(drawContext, pForwardTarget->GetNormal().get());
		if (m_pSelect && m_ui.animation) {
			m_pCameraController->RotateAnimation(m_diff, m_pSelect->CalcCameraFitBox());
		}


		m_diff += timer.Stop();
		m_pRoot->Update(m_diff);
		if (m_diff > 100000.0) { m_diff = 0.0f; }
		glFlush();
		render.Stop();

		m_cpuProfiler.Stop();
		//cpuProfiler.Output();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ui.SetViewport(m_windowSize);
		ShowUI(ui);
		//postEffect.ShowUI(ui);
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

void PointCloudApp::ShowUI(UIContext& ui)
{
	ImVec2 mainPos = ImGui::GetWindowPos();  // 現在のウィンドウの座標
	ImVec2 mainSize = ImGui::GetWindowSize(); // 現在のウィンドウのサイズ
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

	ImGui::Begin("NodeTree");
	ui.SetRoot(UIContext::UIRect(Vector2i(mainPos.x, mainPos.y), Vector2i(mainSize.x, mainSize.y)));
	if (ImGui::TreeNodeEx(m_pRoot->GetName().data(), ImGuiTreeNodeFlags_DefaultOpen)) {
		for (const auto& child : m_pRoot->GetChild()) {
			auto open = ImGui::TreeNode(child.first.data());
			if (ImGui::IsItemClicked()) {
				m_pSelect = child.second.get();
			}

			if (open) {
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
		if (m_ui.animation && m_pSelect) {
			m_pCameraController->FitToBDB(m_pSelect->CalcCameraFitBox());
		}
	}
	ImGui::Checkbox("VisibleSkyBox", &m_ui.visibleSkyBox);
	if (m_pSelect) {
		ImGui::SetNextWindowPos(ImVec2(ui.GetRoot().GetLeftBottom().x, ui.GetRoot().GetLeftBottom().y), ImGuiCond_Always);
		ImGui::Begin(m_pSelect->GetName().data());
		m_pSelect->ShowUIData(ui);
		ImGui::End();
	}


	ImGui::Checkbox("PickMode", &m_ui.pickMode);
	if (m_ui.pickMode) {
		ImGui::SetNextWindowPos(ImGui::GetMousePos());
		ImGui::BeginTooltip();
		for (auto& result : m_pick.pResult) {
			ImGui::Text(result.second->ToString().data());
		}
		auto mousePos = ImGui::GetMousePos();
		String screenPos = "Screen Pos" + glmUtil::ToString(Vector2(mousePos.x,mousePos.y));
		String worldPos = "World Pos" + glmUtil::ToString(m_pick.pickPos);
		ImGui::Text(screenPos.data());
		ImGui::Text(worldPos.data());
		ImGui::EndTooltip();
	}


	auto pCamera = m_pResource->GetCamera();
	ImGui::Text(
		"Eye:(%lf,%lf,%lf)\nCenter:(%lf,%lf,%lf)\nUp:(%lf,%lf,%lf)\n",
		pCamera->Eye().x, pCamera->Eye().y, pCamera->Eye().z,
		pCamera->Center().x, pCamera->Center().y, pCamera->Center().z,
		pCamera->Up().x, pCamera->Up().y, pCamera->Up().z);
	if (ImGui::Button("Camera Fit\n")) {
		auto bdb = m_pRoot->CalcCameraFitBox();
		if (m_pSelect) {
			bdb = m_pSelect->CalcCameraFitBox();
		}
		if (bdb.IsActive()) {
			m_pCameraController->FitToBDB(bdb);
		}
	}
	ImGui::Checkbox("Animation", &m_ui.animation);

	auto pLight = m_pResource->GetLight();
	Vector3 color = pLight->GetColor();
	if (ImGui::ColorEdit3("Light Color", &color[0])) {
		pLight->SetColor(color);
	}



	ImGui::Text("Direction:(%lf, %lf, %lf)\n", pLight->GetDirection().x, pLight->GetDirection().y, pLight->GetDirection().z);
	ImGui::Text("Milli %f, FPS %f", m_cpuProfiler.GetMilli(), m_cpuProfiler.GetFPS());
	static RollingBuffer  ui_fpsDraw, ui_fps60, ui_fps120;
	static float timeDelta = 0.0f;
	ui_fpsDraw.Span = 2.0f;
	ui_fps60.Span = 2.0f;
	ui_fps120.Span = 2.0f;
	timeDelta += ImGui::GetIO().DeltaTime;
	ui_fpsDraw.AddPoint(timeDelta, m_cpuProfiler.GetFPS());
	ui_fps60.AddPoint(timeDelta, 60.0f);
	ui_fps120.AddPoint(timeDelta, 120.0f);
	ImPlot::SetNextPlotLimitsX(0, 2.0f, ImGuiCond_Always);
	ImPlot::SetNextPlotLimitsY(0, 150, ImGuiCond_Always);
	if (ImPlot::BeginPlot("##Rolling", NULL, NULL, ImVec2(-1, 150), 0, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels)) {
		ImPlot::PlotLine("Draw FPS", &ui_fpsDraw.Data[0].x, &ui_fpsDraw.Data[0].y, ui_fpsDraw.Data.size(), 0, 2 * sizeof(float));
		ImPlot::PlotLine("60 FPS", &ui_fps60.Data[0].x, &ui_fps60.Data[0].y, ui_fps60.Data.size(), 0, 2 * sizeof(float));
		ImPlot::PlotLine("120 FPS", &ui_fps120.Data[0].x, &ui_fps120.Data[0].y, ui_fps120.Data.size(), 0, 2 * sizeof(float));
		ImPlot::EndPlot();
	}

	ImGui::End();
}

void PointCloudApp::Finalize()
{
	m_pRoot.reset();
	glfwTerminate();
}

Shared<RenderNode> PointCloudApp::CreateSpaceTest()
{
	auto pNode = std::shared_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\Sponza\\glTF\\Sponza.gltf"));
	pNode->SetScale(100);
	pNode->SetRotateAngle(Vector3(180, 180, 0));
	return pNode;
}
Shared<RenderNode> PointCloudApp::CreateGLTFAnimationTest()
{
	auto pNode = std::shared_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\BrainStem\\glTF\\BrainStem.gltf"));
	pNode->SetScale(50);
	pNode->SetTranslate(Vector3(500, 0, 0));
	return pNode;
}
Shared<RenderNode> PointCloudApp::CreateGLTFNodeTest()
{
	auto pNode = std::shared_ptr<RenderNode>(GLTFLoader::Load("E:\\cgModel\\glTF-Sample-Models-master\\2.0\\DamagedHelmet\\glTF\\DamagedHelmet.gltf"));
	pNode->SetScale(100);
	pNode->SetRotateAngle(Vector3(0, 90, -90));
	pNode->SetTranslate(Vector3(-1000, 100, 0));
	return pNode;
}

Shared<RenderNode> PointCloudApp::CreateLargePointCloudNodeTest()
{
	auto pPointCloud = (Shared<PointCloud>(PointCloudIO::Load("E:\\cgModel\\pointCloud\\bildstein_station3_xyz_intensity_rgb.xyz")));
	auto pNode =(std::make_shared<PointCloudNode>("PointCloud", pPointCloud));
	pNode->SetRotateAngle(Vector3(-90, 0, 0));
	return pNode;
}
Shared<RenderNode> PointCloudApp::CreateCSFNodeTest()
{
	//auto pNode = std::shared_ptr<RenderNode>(CSFLoader::Load("E:\\cgModel\\nv_pro\\downloaded_resources\\blade.csf.gz"));
	auto pNode = std::shared_ptr<RenderNode>(CSFLoader::Load("E:\\cgModel\\nv_pro\\downloaded_resources\\geforce.csf.gz"));
	//auto pNode = std::shared_ptr<RenderNode>(CSFLoader::Load("E:\\cgModel\\nv_pro\\downloaded_resources\\worldcar.csf.gz"));
	//auto pNode = std::shared_ptr<RenderNode>(CSFLoader::Load("E:\\cgModel\\nv_pro\\downloaded_resources\\SubMarine_134.bk3d.gz"));
	pNode->SetScale(500);
	pNode->SetRotateAngle(Vector3(0, 90, 0));
	pNode->SetTranslate(Vector3(-500, 0, 0));
	return pNode;
}
Shared<HalfEdgeNode> PointCloudApp::CreateBunnyNodeTest()
{
	//String path = "E:\\cgModel\\Armadillo.half";
	String path = "E:\\cgModel\\bunny6000.half";
	auto data = std::shared_ptr<HalfEdgeStruct>(HalfEdgeLoader::Load(path));
	auto node = std::make_shared<HalfEdgeNode>(path, data);
	node->SetMatrix(glmUtil::CreateRotate(glm::pi<float>() / 2, Vector3(0, 0, 1)));
	return node;
}

Shared<RenderNode> PointCloudApp::CreateSTEPNodeTest()
{
	auto pNode = std::shared_ptr<RenderNode>(STEPLoader::Load("E:\\cgModel\\step\\123Block_Color.stp"));
	//auto pNode = std::shared_ptr<RenderNode>(STEPLoader::Load("E:\\cgModel\\step\\cube.stp"));
	return pNode;
}

Shared<PointCloudNode> PointCloudApp::CreateDelaunayTest()
{
	auto pos2D = PointCloud::Create3D(250, Vector3(-100, -100, 0), Vector3(100, 100, 0));
	auto point2DPrim = std::make_shared<PointCloud>();
	point2DPrim->SetPosition(std::move(pos2D));
	point2DPrim->SetType(GL_POINTS);
	return std::make_shared<PointCloudNode>("2DPointCloud", point2DPrim);
}
Shared<InstancedPrimitiveNode> PointCloudApp::CreateInstacedNodeTest()
{
	Shared<Primitive> pCube = std::make_shared<Cube>(vec3(0, 0, 0), vec3(10, 10, 10));

	auto pNode = std::make_shared<InstancedPrimitiveNode>("InstancedCube", pCube, Vector3(1, 0, 0));
	Vector<Matrix4x4> matrixs;
	matrixs.push_back(glmUtil::CreateTranslate(Vector3(15, 0, 0)));
	matrixs.push_back(glmUtil::CreateTranslate(Vector3(0, 15, 0)));
	matrixs.push_back(glmUtil::CreateTranslate(Vector3(0, 0, 15)));
	pNode->SetMatrixs(std::move(matrixs));
	return pNode;
}
}
#include "SoftwareRasterizer.h"
#include "Texture.h"
#include "PostEffect.h"
#include "Utility.h"
#include "GeometryUtility.h"
#include "HalfEdgeLoader.h"
namespace KI
{
void SoftwareRasterizer::Initialize()
{
	GLFWApp::Initialize();
	String path = "E:\\cgModel\\bunny6000.half";
	m_pBunny = std::shared_ptr<HalfEdgeStruct>(HalfEdgeLoader::Load(path));
	m_pCameraController->FitToBDB(BDB(m_pBunny->GetVertex()));
	m_colorPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	m_depthPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	m_debugPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	m_pColorTexture = std::make_shared<Texture2D>();
	m_pDepthTexture = std::make_shared<Texture2D>();

}

void SoftwareRasterizer::Execute()
{
	GLuint VertexArrayID;
	auto m_pResource = std::make_unique<RenderResource>();
	m_pResource->Build();
	DrawContext context(m_pResource.get());
	Rasterize();

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 400 core");


	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glViewport(0, 0, m_windowSize.x, m_windowSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (m_ui.showDepth) {
			TextureDrawer::Execute(context, m_pDepthTexture.get());
		} else {
			TextureDrawer::Execute(context, m_pColorTexture.get());
		}
	
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::Checkbox("ShowDepth", &m_ui.showDepth);
		ImGui::Checkbox("PickMode", &m_ui.pickMode);
		if (m_ui.pickMode) {
			ImGui::SetNextWindowPos(ImGui::GetMousePos());
			ImGui::BeginTooltip();
			auto mousePos = ImGui::GetMousePos();
			String screenPos = "Screen Pos : " + glmUtil::ToString(Vector2(mousePos.x, m_windowSize.y - mousePos.y));
			String depthPos = "Depth Pos : " + IntToString(m_debugPixel.Get(mousePos.x, m_windowSize.y - mousePos.y).x);
			String triangle = "Triangle Pos : " + IntToString(m_debugPixel.Get(mousePos.x, m_windowSize.y - mousePos.y).y);
			ImGui::Text(screenPos.data());
			ImGui::Text(depthPos.data());
			ImGui::Text(triangle.data());
			ImGui::EndTooltip();
		}

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(m_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}
	glDeleteVertexArrays(1, &VertexArrayID);
}

void SoftwareRasterizer::ProcessMouseEvent(const MouseInput& input)
{
	m_pMouse->ApplyMouseInput(input);
	EditContext context(m_pMouse.get(), m_pCamera.get());
	if (input.Event() == MOUSE_EVENT_WHEEL) {
		m_pCameraController->Wheel(context);
		Rasterize();
	} else if (input.Event() == MOUSE_EVENT_MOVE) {
		m_pCameraController->Move(context);
	}

	if (m_pMouse->Press(MOUSE_BUTTON_LEFT)) {
		if (m_ui.pickMode) {
			Printf(m_pMouse->Position().x, m_windowSize.y - m_pMouse->Position().y);
		}
	}

	if (m_pMouse->Press(MOUSE_BUTTON_RIGHT) ||
		m_pMouse->Press(MOUSE_BUTTON_MIDDLE)) {
		Rasterize();
	}

}
void SoftwareRasterizer::Printf(int x, int y)
{
	auto depth = m_debugPixel.Get(x, y).x;
	auto triangleIndex = m_debugPixel.Get(x, y).y;
	auto vp = m_pCamera->GetViewProj();
	auto model = Matrix4x4(1.0);
	auto tri = m_pBunny->GetFace(triangleIndex);
	Vector3 screen0;
	Vector3 screen1;
	Vector3 screen2;
	if (!MathHelper::ToScreen(m_pCamera->GetViewport(), vp, model, tri.pos0, screen0)) { return; }
	if (!MathHelper::ToScreen(m_pCamera->GetViewport(), vp, model, tri.pos1, screen1)) { return; }
	if (!MathHelper::ToScreen(m_pCamera->GetViewport(), vp, model, tri.pos2, screen2)) { return; }

	auto area = GeometryUtility::CalcArea(screen0, screen1, screen2);
	DebugPrintf::StringStr("---------------\n");
	DebugPrintf::Int("Face : ", triangleIndex); DebugPrintf::NewLine();
	DebugPrintf::Vec3(screen0); DebugPrintf::NewLine();
	DebugPrintf::Vec3(screen1); DebugPrintf::NewLine();
	DebugPrintf::Vec3(screen2); DebugPrintf::NewLine();
	DebugPrintf::Float("Depth : ", depth); DebugPrintf::NewLine();
	DebugPrintf::Float("Area : ", area); DebugPrintf::NewLine();
	DebugPrintf::StringStr("---------------\n");
}
void SoftwareRasterizer::ResizeEvent(int width, int height)
{
	m_windowSize = Vector2i(width, height);
	m_colorPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	m_depthPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	m_debugPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	if (m_pCamera) {
		m_pCameraController->SetAspect(m_windowSize.x, m_windowSize.y);
		m_pCamera->SetViewport(Vector4i(0, 0, width, height));
	}
	Rasterize();
}


bool SoftwareRasterizer::IsDrawTriangle(const Vector3& screen0, const Vector3& screen1, const Vector3& screen2, float area, const Vector2& target, float& targetZ)
{
	auto w0 = glmUtil::Cross(target - Vector2(screen1), screen2 - screen1) / area;
	auto w1 = glmUtil::Cross(target - Vector2(screen2), screen0 - screen2) / area;
	auto w2 = glmUtil::Cross(target - Vector2(screen0), screen1 - screen0) / area;

	bool in =
		(w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) ||
		(w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f);

	if (in) {
		targetZ = w0 * screen0.z + w1 * screen1.z + w2 * screen2.z;
		return true;
	} else {
		return false;
	}
}
void SoftwareRasterizer::Rasterize()
{
	auto vp = m_pCamera->GetViewProj();
	auto model = Matrix4x4(1.0);
	m_colorPixel.Fill(0);
	m_depthPixel.Fill(255.0f);
	m_debugPixel.Fill(0);
	Vector<Vector3> position;
	position.push_back(Vector3(-0.5f, -0.5f, 0));
	position.push_back(Vector3(0.5f, -0.5f, 0));
	position.push_back(Vector3(0.0f, 0.5f, 0));
	auto mesh = Mesh(std::move(position), Mesh::DrawType::Triangles);
	//for (size_t k = 0; k < mesh.TriangleNum(); k++) {
	//	auto tri = mesh.GetTriangle(k);
	//	vp = Matrix4x4(1.0);
	for (size_t k = 0; k < m_pBunny->GetFaceNum(); k++) {
		auto tri = m_pBunny->GetFace(k);
		auto normal = m_pBunny->CalcFaceNormal(k);
		Vector3 screen0;
		Vector3 screen1;
		Vector3 screen2;
		if (!MathHelper::ToScreen(m_pCamera->GetViewport(), vp, model, tri.pos0, screen0)) { continue; }
		if (!MathHelper::ToScreen(m_pCamera->GetViewport(), vp, model, tri.pos1, screen1)) { continue; }
		if (!MathHelper::ToScreen(m_pCamera->GetViewport(), vp, model, tri.pos2, screen2)) { continue; }
		auto area = GeometryUtility::CalcArea(screen0, screen1, screen2);
		if (area <= 0.0f) { continue; }

		Rangef xRange, yRange;
		xRange.Add(screen0.x); xRange.Add(screen1.x); xRange.Add(screen2.x);
		yRange.Add(screen0.y); yRange.Add(screen1.y); yRange.Add(screen2.y);

		int minX = std::max(0, xRange.RoundMin());
		int maxX = std::min(m_windowSize.x - 1, xRange.RoundMax());
		int minY = std::max(0, yRange.RoundMin());
		int maxY = std::min(m_windowSize.y - 1, yRange.RoundMax());


		for (int i = minX; i < maxX; i++) {
			for (int j = minY; j < maxY; j++) {
				float targetZ = 0.0f;
				Vector2 target{ i + 0.5f, j + 0.5f };
				if (IsDrawTriangle(screen0, screen1, screen2, area, target, targetZ)) {
					if (m_depthPixel.Get(i, j).x > targetZ * 255.0f) {
						// depth, triangle Index
						m_debugPixel.Set(i, j, Vector4(targetZ * 255.0f, k, 0, 0));
						m_depthPixel.Set(i, j, Vector4(targetZ * 255.0f, targetZ * 255.0f, targetZ * 255.0f, 255.0f));
						m_colorPixel.Set(i, j, Vector4(normal * 255.0f, 255.0f));
					}
				}
			}
		}
	}

	m_pColorTexture->Build(m_colorPixel.width, m_colorPixel.height, m_colorPixel.data);
	m_pDepthTexture->Build(m_depthPixel.width, m_depthPixel.height, m_depthPixel.data);
}
void SoftwareRasterizer::Finalize()
{
	m_pColorTexture = nullptr;
	m_pDepthTexture = nullptr;
	GLFWApp::Finalize();
}
}
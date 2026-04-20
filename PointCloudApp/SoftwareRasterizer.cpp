#include "SoftwareRasterizer.h"
#include "Texture.h"
#include "PostEffect.h"
#include "Utility.h"
#include "GeometryUtility.h"
#include "HalfEdgeLoader.h"
#include "Profiler.h"
#include "tbb/parallel_for.h"
namespace KI
{
void SoftwareRasterizer::Initialize()
{
	GLFWApp::Initialize();
	String path = "E:\\cgModel\\bunny6000.half";
	m_pBunny = std::shared_ptr<HalfEdgeStruct>(HalfEdgeLoader::Load(path));
	m_pCameraController->FitToBDB(BDB(m_pBunny->GetVertex()));
	m_colorPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	m_depthPixel.Allocate(m_windowSize.x, m_windowSize.y, 1);
	m_debugPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	m_pColorTexture = std::make_shared<Texture2D>();
	m_pDepthTexture = std::make_shared<Texture2D>();
}


bool IsDrawTriangle(const Vector3& screen0, const Vector3& screen1, const Vector3& screen2, float invArea, const Vector2& target, float& targetZ)
{
	auto w0 = glmUtil::Cross(target - Vector2(screen1), screen2 - screen1) * invArea;
	auto w1 = glmUtil::Cross(target - Vector2(screen2), screen0 - screen2) * invArea;
	auto w2 = glmUtil::Cross(target - Vector2(screen0), screen1 - screen0) * invArea;

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
struct EdgeFunction
{
	Vector3 edge = Vector3();
	float Evaluate(float x, float y) const
	{
		return edge.x * x + edge.y * y + edge.z;
	}

	EdgeFunction(const Vector3& v0, const Vector3& v1)
	{
		edge.x = v0.y - v1.y;
		edge.y = v1.x - v0.x;
		edge.z = v0.x * v1.y - v1.x * v0.y;
	}
};



struct TBB_Function
{
	struct Args
	{
		HalfEdgeStruct* pBunny;
		Camera* pCamera;
		Matrix4x4 mvp;
		Vector2i windowSize;
		PixelDataf* colorPixel;
		PixelDataf* depthPixel;
		PixelDataf* debugPixel;
	};
	TBB_Function(const Args& args) : m_args(args) {}
	void operator()(tbb::blocked_range<int>& r) const
	{
		for (int k = r.begin(); k != r.end(); ++k) {
			auto tri = m_args.pBunny->GetFace(k);
			const auto& normal = m_args.pBunny->GetFaceNormal()[k];
			Vector3 screen0;
			Vector3 screen1;
			Vector3 screen2;
			if (!MathHelper::ToScreen(m_args.pCamera->GetViewport(), m_args.mvp, tri.pos0, screen0)) { continue; }
			if (!MathHelper::ToScreen(m_args.pCamera->GetViewport(), m_args.mvp, tri.pos1, screen1)) { continue; }
			if (!MathHelper::ToScreen(m_args.pCamera->GetViewport(), m_args.mvp, tri.pos2, screen2)) { continue; }


			{
				auto e0 = EdgeFunction(screen1, screen2);
				auto e1 = EdgeFunction(screen2, screen0);
				auto e2 = EdgeFunction(screen0, screen1);
				float area = e2.Evaluate(screen2.x, screen2.y);
				if (area <= 0.0f) { continue; }
				area = 1 / area;

				Rangef xRange, yRange;
				xRange.Add(screen0.x); xRange.Add(screen1.x); xRange.Add(screen2.x);
				yRange.Add(screen0.y); yRange.Add(screen1.y); yRange.Add(screen2.y);

				int minX = std::max(0, xRange.RoundMin());
				int maxX = std::min(m_args.windowSize.x - 1, xRange.RoundMax());
				int minY = std::max(0, yRange.RoundMin());
				int maxY = std::min(m_args.windowSize.y - 1, yRange.RoundMax());

				
				float w0_raw = e0.Evaluate(minX, minY); 
				float w1_raw = e1.Evaluate(minX, minY); 
				float w2_raw = e2.Evaluate(minX, minY);
				for (int i = minX; i < maxX; i++) {
					for (int j = minY; j < maxY; j++) {
						const bool in =
							(w0_raw >= 0.0f && w1_raw >= 0.0f && w2_raw >= 0.0f) ||
							(w0_raw <= 0.0f && w1_raw <= 0.0f && w2_raw <= 0.0f);
						if (in) {
							float w0 = w0_raw * area;
							float w1 = w1_raw * area;
							float w2 = w2_raw * area;

							float targetZ = w0 * screen0.z + w1 * screen1.z + w2 * screen2.z;
							targetZ = pow(targetZ, 50.0f);
							if (m_args.depthPixel->GetR(i, j) > targetZ) {
								m_args.debugPixel->Set(i, j, Vector4(k, 0, 0, 0));
								m_args.depthPixel->Set(i, j, Vector4(targetZ, targetZ, targetZ, 255.0f));
								m_args.colorPixel->Set(i, j, Vector4(normal, 255.0f));
							}
						}
						w0_raw += e0.edge.y;
						w1_raw += e1.edge.y;
						w2_raw += e2.edge.y;
					}
					w0_raw += e0.edge.x;
					w1_raw += e1.edge.x;
					w2_raw += e2.edge.x;
				}

				continue;
			}



			auto area = GeometryUtility::CalcArea(screen0, screen1, screen2);
			if (area <= 0.0f) { continue; }

			Rangef xRange, yRange;
			xRange.Add(screen0.x); xRange.Add(screen1.x); xRange.Add(screen2.x);
			yRange.Add(screen0.y); yRange.Add(screen1.y); yRange.Add(screen2.y);

			int minX = std::max(0, xRange.RoundMin());
			int maxX = std::min(m_args.windowSize.x - 1, xRange.RoundMax());
			int minY = std::max(0, yRange.RoundMin());
			int maxY = std::min(m_args.windowSize.y - 1, yRange.RoundMax());

			for (int i = minX; i < maxX; i++) {
				for (int j = minY; j < maxY; j++) {
					float targetZ = 0.0f;
					Vector2 target{ i + 0.5f, j + 0.5f };
					if (IsDrawTriangle(screen0, screen1, screen2, area, target, targetZ)) {
						targetZ = pow(targetZ, 50.0); // 誇張
						if (m_args.depthPixel->GetR(i, j) > targetZ) {
							// triangle Index
							m_args.debugPixel->Set(i, j, Vector4(k, 0, 0, 0));
							m_args.depthPixel->Set(i, j, targetZ);
							m_args.colorPixel->Set(i, j, Vector4(normal, 255.0f));
						}
					}
				}
			}
		}
	}
	Args m_args;
};

// コールバック関数を定義
void APIENTRY MyGLDebugCallback1(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam)
{
	fprintf(stderr, "GL DEBUG: %s\n", message);
}

void SoftwareRasterizer::Execute()
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(MyGLDebugCallback1, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

	GLuint VertexArrayID;
	auto m_pResource = std::make_unique<RenderResource>();
	m_pResource->Build();
	DrawContext context(m_pResource.get());
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	m_pColorTexture->Allocate(Texture2D::CreateRGBAF(m_windowSize.x, m_windowSize.y));
	m_pDepthTexture->Allocate(Texture2D::CreateRGBAF(m_windowSize.x, m_windowSize.y));
	Rasterize();

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
		
		if (ImGui::Checkbox("UseTBB", &m_ui.useTBB)) {
			Rasterize();
		}
		if (ImGui::SliderInt("TBBGrainSize", &m_ui.tbbGrainSize, 0, 10)) {
			Rasterize();
		}

		if (ImGui::Checkbox("ShowDepth", &m_ui.showDepth)) {
			Rasterize();
		}
		ImGui::Checkbox("PickMode", &m_ui.pickMode);
		if (m_ui.pickMode) {
			ImGui::SetNextWindowPos(ImGui::GetMousePos());
			ImGui::BeginTooltip();
			auto mousePos = ImGui::GetMousePos();
			String screenPos = "Screen Pos : " + glmUtil::ToString(Vector2(mousePos.x, m_windowSize.y - mousePos.y));
			String triangle = "Triangle Pos : " + IntToString(m_debugPixel.Get(mousePos.x, m_windowSize.y - mousePos.y).x);
			ImGui::Text(screenPos.data());
			ImGui::Text(triangle.data());
			ImGui::EndTooltip();
		}

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(m_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);
		glfwPollEvents();
		OUTPUT_GLERROR;
	}
	ImGui::DestroyContext();
	glDeleteVertexArrays(1, &VertexArrayID);
}

void SoftwareRasterizer::ProcessMouseEvent(const MouseInput& input)
{
	m_pMouse->ApplyMouseInput(input);
	EditContext context(m_pMouse.get(), m_pCamera.get());
	if (input.Event() == MOUSE_EVENT_WHEEL) {
		m_pCameraController->Wheel(context);
	} else if (input.Event() == MOUSE_EVENT_MOVE) {
		m_pCameraController->Move(context);
	}

	if (m_pMouse->Press(MOUSE_BUTTON_LEFT)) {
		if (m_ui.pickMode) {
			Printf(m_pMouse->Position().x, m_windowSize.y - m_pMouse->Position().y);
		}
	}

	if (m_pMouse->Press(MOUSE_BUTTON_RIGHT) ||
		m_pMouse->Press(MOUSE_BUTTON_MIDDLE) ||
		input.Event() == MOUSE_EVENT_WHEEL) {
		CPUProfiler profiler;
		profiler.Start();
		Rasterize();
		profiler.Stop();
		profiler.Output();
	}

}
void SoftwareRasterizer::Printf(int x, int y)
{
	auto depth = m_depthPixel.Get(x, y).x;
	auto triangleIndex = m_debugPixel.Get(x, y).x;
	auto normal = m_pBunny->CalcFaceNormal(triangleIndex);
	auto vp = m_pCamera->GetViewProj();
	auto model = Matrix4x4(1.0);
	auto tri = m_pBunny->GetFace(triangleIndex);
	Matrix4x4 mvp = vp * model;

	Vector3 screen0;
	Vector3 screen1;
	Vector3 screen2;
	if (!MathHelper::ToScreen(m_pCamera->GetViewport(), mvp, tri.pos0, screen0)) { return; }
	if (!MathHelper::ToScreen(m_pCamera->GetViewport(), mvp, tri.pos1, screen1)) { return; }
	if (!MathHelper::ToScreen(m_pCamera->GetViewport(), mvp, tri.pos2, screen2)) { return; }

	auto area = GeometryUtility::CalcArea(screen0, screen1, screen2);
	DebugPrintf::StringStr("---------------\n");
	DebugPrintf::Int("Face : ", triangleIndex); DebugPrintf::NewLine();
	DebugPrintf::Vec3("Screen0", screen0); DebugPrintf::NewLine();
	DebugPrintf::Vec3("Screen1", screen1); DebugPrintf::NewLine();
	DebugPrintf::Vec3("Screen2", screen2); DebugPrintf::NewLine();
	DebugPrintf::Vec3("Normal", normal); DebugPrintf::NewLine();
	DebugPrintf::Float("Depth : ", depth * 1000); DebugPrintf::NewLine();
	DebugPrintf::Float("Area : ", area); DebugPrintf::NewLine();
	DebugPrintf::StringStr("---------------\n");
}
void SoftwareRasterizer::ResizeEvent(int width, int height)
{
	m_windowSize = Vector2i(width, height);
	m_colorPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	m_depthPixel.Allocate(m_windowSize.x, m_windowSize.y, 1);
	m_debugPixel.Allocate(m_windowSize.x, m_windowSize.y, 4);
	m_pColorTexture->Allocate(Texture2D::CreateRGBAF(m_windowSize.x, m_windowSize.y));
	m_pDepthTexture->Allocate(Texture2D::CreateRGBAF(m_windowSize.x, m_windowSize.y));
	if (m_pCamera) {
		m_pCameraController->SetAspect(m_windowSize.x, m_windowSize.y);
		m_pCamera->SetViewport(Vector4i(0, 0, width, height));
	}
	Rasterize();
}

void SoftwareRasterizer::Rasterize()
{
	auto vp = m_pCamera->GetViewProj();
	auto model = Matrix4x4(1.0);
	m_colorPixel.Fill(0);
	m_depthPixel.Fill(INFINITY);
	m_debugPixel.Fill(0);

	TBB_Function::Args args;
	args.mvp = vp * model;
	args.pCamera = m_pCamera.get();
	args.pBunny = m_pBunny.get();
	args.windowSize = m_windowSize;
	args.colorPixel = &m_colorPixel;
	args.depthPixel = &m_depthPixel;
	args.debugPixel = &m_debugPixel;

	if(m_ui.useTBB)
	{
		CPUProfiler profiler;
		profiler.Start();
		tbb::parallel_for(tbb::blocked_range<int>(0, m_pBunny->GetFaceNum(), 1 << m_ui.tbbGrainSize), TBB_Function(args));
		profiler.Stop();
		profiler.Output();
		if (m_ui.showDepth) {
			m_pDepthTexture->Update(m_depthPixel.data);
		} else {
			m_pColorTexture->Update(m_colorPixel.data);
		}
		return;
	} 

	Vector<Vector3> position;
	position.push_back(Vector3(-0.5f, -0.5f, 0));
	position.push_back(Vector3(0.5f, -0.5f, 0));
	position.push_back(Vector3(0.0f, 0.5f, 0));
	auto mesh = Mesh(std::move(position), Mesh::DrawType::Triangles);
	//for (size_t k = 0; k < mesh.TriangleNum(); k++) {
	//	auto tri = mesh.GetTriangle(k);
	//	vp = Matrix4x4(1.0);
	Matrix4x4 mvp = vp * model;
	CPUProfiler profiler;
	profiler.Start();
	for (size_t k = 0; k < m_pBunny->GetFaceNum(); k++) {
		auto tri = m_pBunny->GetFace(k);
		const auto& normal = m_pBunny->GetFaceNormal()[k];
		Vector3 screen0;
		Vector3 screen1;
		Vector3 screen2;
		if (!MathHelper::ToScreen(m_pCamera->GetViewport(), mvp, tri.pos0, screen0)) { continue; }
		if (!MathHelper::ToScreen(m_pCamera->GetViewport(), mvp, tri.pos1, screen1)) { continue; }
		if (!MathHelper::ToScreen(m_pCamera->GetViewport(), mvp, tri.pos2, screen2)) { continue; }
		
		//{
		//	auto e0 = EdgeFunction(screen1, screen2);
		//	auto e1 = EdgeFunction(screen2, screen0);
		//	auto e2 = EdgeFunction(screen0, screen1);
		//	float area = e2.Evaluate(screen2.x, screen2.y);
		//	if (area <= 0.0f) { continue; }
		//	area = 1 / area;

		//	Rangef xRange, yRange;
		//	xRange.Add(screen0.x); xRange.Add(screen1.x); xRange.Add(screen2.x);
		//	yRange.Add(screen0.y); yRange.Add(screen1.y); yRange.Add(screen2.y);

		//	int minX = std::max(0, xRange.RoundMin());
		//	int maxX = std::min(m_windowSize.x - 1, xRange.RoundMax());
		//	int minY = std::max(0, yRange.RoundMin());
		//	int maxY = std::min(m_windowSize.y - 1, yRange.RoundMax());


		//	float w0_tri = e0.Evaluate(minX + 0.5f, minY + 0.5f);
		//	float w1_tri = e1.Evaluate(minX + 0.5f, minY + 0.5f);
		//	float w2_tri = e2.Evaluate(minX + 0.5f, minY + 0.5f);
		//	for (int i = minX; i < maxX; i++) {
		//		float w0_raw = w0_tri + (i - minX) * e0.edge.x;
		//		float w1_raw = w1_tri + (i - minX) * e1.edge.x;
		//		float w2_raw = w2_tri + (i - minX) * e2.edge.x;

		//		for (int j = minY; j < maxY; j++) {
		//			if (w0_raw >= 0.0f && w1_raw >= 0.0f && w2_raw >= 0.0f) {
		//				float targetZ = 
		//					w0_raw * area * screen0.z + 
		//					w1_raw * area * screen1.z + 
		//					w2_raw * area * screen2.z;

		//				targetZ = pow(targetZ, 50.0f);
		//				if (m_depthPixel.GetR(i, j) > targetZ) {
		//					m_debugPixel.Set(i, j, Vector4(k, 0, 0, 0));
		//					m_depthPixel.Set(i, j, Vector4(targetZ, targetZ, targetZ, 255.0f));
		//					m_colorPixel.Set(i, j, Vector4(normal, 255.0f));
		//				}
		//			}
		//			w0_raw += e0.edge.y;
		//			w1_raw += e1.edge.y;
		//			w2_raw += e2.edge.y;
		//		}
		//	}

		//	continue;
		//}
		
		
		auto area = GeometryUtility::CalcArea(screen0, screen1, screen2);
		if (area <= 0.0f) { continue; }
		auto invArea = 1 / area;

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
				if (IsDrawTriangle(screen0, screen1, screen2, invArea, target, targetZ)) {
					targetZ = pow(targetZ, 50.0); // 誇張
					if (m_depthPixel.GetR(i, j) > targetZ) {
						// triangle Index
						m_debugPixel.Set(i, j, Vector4(k, 0, 0, 0));
						m_depthPixel.Set(i, j, targetZ);
						m_colorPixel.Set(i, j, Vector4(normal, 255.0f));
					}
				}
			}
		}
	}
	profiler.Stop();
	profiler.Output();
	if (m_ui.showDepth) {
		m_pDepthTexture->Update(m_depthPixel.data);
	} else {
		m_pColorTexture->Update(m_colorPixel.data);
	}
}
void SoftwareRasterizer::Finalize()
{
	m_pColorTexture = nullptr;
	m_pDepthTexture = nullptr;
	GLFWApp::Finalize();
}
}
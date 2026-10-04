#include "App/STEPViewer.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include "Node/STEP/STEPNode.h"
#include "Utility/Profiler.h"

namespace KI
{
void STEPViewer::InitializeFiles()
{
	m_uiContext.ClearDebugNode();
	m_pSTEPNode.reset();
	m_nodes.clear();
	m_files.clear();
	enum STEP_FOLDER
	{
		largeData,
		ap224,
		ap214,
		ap209,
		ap203e2,
		ap203,
		finish,
		other
	};

	STEP_FOLDER folder = static_cast<STEP_FOLDER>(m_folder);
	{
		if (folder == largeData) {
			m_files.push_back("E:\\cgModel\\step\\largeData\\Ai-14R.stp");
			m_files.push_back("E:\\cgModel\\step\\largeData\\Cruise_Assembly.stp");
			m_files.push_back("E:\\cgModel\\step\\largeData\\NissanGT-R.STEP");
			m_files.push_back("E:\\cgModel\\step\\largeData\\Rocky_House.stp");
			m_files.push_back("E:\\cgModel\\step\\largeData\\ROTOR-201NAL-Z7.STEP");
			m_files.push_back("E:\\cgModel\\step\\largeData\\Scania-8x4.stp");
			m_files.push_back("E:\\cgModel\\step\\largeData\\Scania-Engine-V8-XT-Turbo.step");
			m_files.push_back("E:\\cgModel\\step\\largeData\\UMC-500_SS_Solid_Model_2019-06_r1.stp");
		} else if (folder == ap224) {
			m_files.push_back("E:\\cgModel\\step\\ap224\\ap224_995288709.stp");
			m_files.push_back("E:\\cgModel\\step\\ap224\\ap224_995315479.stp");
			m_files.push_back("E:\\cgModel\\step\\ap224\\ap224_995602415.stp");
			m_files.push_back("E:\\cgModel\\step\\ap224\\ap224_997423743.stp");
			m_files.push_back("E:\\cgModel\\step\\ap224\\ap224_997865309.stp");
		} else if (folder == ap214)	{
			m_files.push_back("E:\\cgModel\\step\\ap214\\as1-ac-214.stp");
			m_files.push_back("E:\\cgModel\\step\\ap214\\as1-ec-214.stp");
			m_files.push_back("E:\\cgModel\\step\\ap214\\as1-md-214.stp");
			m_files.push_back("E:\\cgModel\\step\\ap214\\as1-tc-214.stp");
			m_files.push_back("E:\\cgModel\\step\\ap214\\as1-ug-214.stp");
		} else if (folder == ap209) {
			m_files.push_back("E:\\cgModel\\step\\ap209\\blower.stp");
			m_files.push_back("E:\\cgModel\\step\\ap209\\mshaft.stp");
		} else if(folder == ap203e2) {
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\123Block_Color.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\123Block_Dimension.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\123Block_Short_Note.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\boxy_with_cylindricity.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\boxy_with_diamsize.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\boxy_with_flatness.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\boxy_with_limitsandfits.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\boxy_with_linearsize.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\boxy_with_perp.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203e2\\boxy_with_surfacetex.stp");
		} else if(folder == ap203) {
			m_files.push_back("E:\\cgModel\\step\\ap203\\1797609in.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\2827056.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\4pinplug.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\53711_74563f01_na.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\angle1.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\as1_pe.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\bernetl.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\block.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\boeing_part.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\boeing_part_simple.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\bracket1-part.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\bull.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\calo.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\calosoe.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\chair.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\clevis21.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\clevis22.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\clevis23.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\clip.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\cubcylso.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\cubsomcy.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\cylcub.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\daratech.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\doghouse.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\filler.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\gehaeuse.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\hose-fitting.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\interacting_pockets.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\iso14649-demo.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\jack_in_the_box.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\lower_carriage.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\mohne.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\monster4.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\moon_buggy.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\moon_buggy_asm.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\mycami2.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\nasty_cheese.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\ph4m3-st.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\piston.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\rear.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\snet.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\socket.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\socks.stp");
			//m_files.push_back("E:\\cgModel\\step\\ap203\\st203-bapl.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\team.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\teampart.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\tork.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\turbine.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\unterlaf.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\upper_carriage.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\vaccase_asm_solid.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\valve.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\vs_training.stp");
			m_files.push_back("E:\\cgModel\\step\\ap203\\weldment_asm_solid.stp");
		} else if (folder == other) {

			//// 実施中
			//m_files.push_back("E:\\cgModel\\step\\turbine.stp");
			//m_files.push_back("E:\\cgModel\\step\\ap224_995288709.stp");

			//// 高難度データ

			//// B-Spline
			//m_files.push_back("E:\\cgModel\\step\\filler.stp");
			//m_files.push_back("E:\\cgModel\\step\\fusion360\\fillet3D.step");
		} else if (folder == finish) {
			// 完成データ
			{
				// CYLINDRICAL_SURFACE
				//m_files.push_back("E:\\cgModel\\step\\angle1.stp");
				//m_files.push_back("E:\\cgModel\\step\\fusion360\\fillet2D.step");
				//m_files.push_back("E:\\cgModel\\step\\interacting_pockets.stp");
				//m_files.push_back("E:\\cgModel\\step\\mycylinder.stp");
				//m_files.push_back("E:\\cgModel\\step\\cubsomcy.stp");
				//m_files.push_back("E:\\cgModel\\step\\123Block_Color.stp");
				//m_files.push_back("E:\\cgModel\\step\\fusion360\\fillet2D.step");
				//m_files.push_back("E:\\cgModel\\step\\fusion360\\Torus2D.step");
				//m_files.push_back("E:\\cgModel\\step\\lower_carriage.stp");
				//m_files.push_back("E:\\cgModel\\step\\cubcylso.stp"); // Alignment
				//m_files.push_back("E:\\cgModel\\step\\bull.stp"); // B-Spline
				//m_files.push_back("E:\\cgModel\\step\\bull_easy.step"); // B-Spline
				//m_files.push_back("E:\\cgModel\\step\\angle1_FACE346_orig.step"); // B-Spline
				//m_files.push_back("E:\\cgModel\\step\\fusion360\\concaveCylinder.step");
				m_files.push_back("E:\\cgModel\\step\\fusion360\\concaveCylinder_FACE64_orig.step");
			}
		}
	}
	m_nodes.resize(m_files.size());
	m_status.assign(m_files.size(), String());
	m_loadAttempted.assign(m_files.size(), false);
	m_selectedFile = 0;
	m_galleryPage = 0;
	UpdateRenderData();
}

void STEPViewer::Initialize()
{
	GLFWApp::Initialize();
	glfwSetWindowTitle(m_window, "STEP Viewer");
	m_pResource = std::make_shared<RenderResource>();
	m_pResource->Build();
	m_pResource->GL()->SetWindowSize(m_windowSize);
	m_pResource->GL()->EnablePolygonOffset(1, 1);
	m_pResource->GL()->SetLineWidth(2.0f);
	m_pResource->GL()->SetPointSize(5.0f);
	m_pResource->SetMainCamera(m_pCamera);
	m_pDebugRoot = std::make_shared<RenderNode>("STEP Debug");
	m_uiContext.SetDebugNode(m_pDebugRoot.get());
	m_uiContext.SetCurrentController(nullptr);
	InitializeFiles();
}

Shared<STEPRenderNode> STEPViewer::LoadSTEP(int index, bool reload)
{
	if (index < 0 || index >= static_cast<int>(m_files.size())) { return nullptr; }
	if (!reload && m_loadAttempted[index]) { return m_nodes[index]; }
	m_loadAttempted[index] = true;
	try {
		const auto& path = m_files[index];
		if (!std::filesystem::is_regular_file(path)) {
			m_status[index] = "STEP file does not exist.";
			return m_nodes[index];
		}
		Shared<STEPStruct> step(STEPLoader::Load(path));
		if (!step) {
			m_status[index] = "Failed to load STEP data (.step / .stp).";
			return m_nodes[index];
		}
		if (step->closedShell.empty() && step->openShell.empty()) {
			m_status[index] = "No supported STEP shells found.";
			return m_nodes[index];
		}
		auto node = std::make_shared<STEPRenderNode>(path, step);
		m_uiContext.ClearDebugNode();
		m_nodes[index] = std::move(node);
		m_elapsed = 0.0f;
		m_status[index] = "Loaded: " + path;
	} catch (const std::exception& error) {
		m_status[index] = "Failed to load STEP: " + String(error.what());
	}
	return m_nodes[index];
}

void STEPViewer::UpdateRenderData()
{
	m_uiContext.ClearDebugNode();
	m_pSTEPNode = LoadSTEP(m_selectedFile);
	if (m_galleryMode) {
		const int first = m_galleryPage * GalleryPageSize;
		const int end = std::min(first + GalleryPageSize, static_cast<int>(m_files.size()));
		for (int i = first; i < end; ++i) { LoadSTEP(i); }
	} else {
		FitCamera();
	}
}

void STEPViewer::DrawGallery(const DrawContext& context)
{
	const auto size = m_pResource->GL()->GetWindowSize();
	if (size.x < GalleryColumns || size.y < GalleryRows) { return; }
	for (int slot = 0; slot < GalleryPageSize; ++slot) {
		const int index = m_galleryPage * GalleryPageSize + slot;
		if (index >= static_cast<int>(m_nodes.size())) { break; }
		const auto& node = m_nodes[index];
		if (!node) { continue; }
		const auto box = node->CalcCameraFitBox();
		if (!std::isfinite(box.MaxLength()) || box.MaxLength() <= 0.0f) { continue; }
		const int col = slot % GalleryColumns;
		const int row = GalleryRows - 1 - slot / GalleryColumns;
		const int x = col * size.x / GalleryColumns;
		const int y = row * size.y / GalleryRows;
		const int width = (col + 1) * size.x / GalleryColumns - x;
		const int height = (row + 1) * size.y / GalleryRows - y;
		m_pResource->GL()->SetViewport(Viewport(Vector4i(x, y, width, height)));
		m_pResource->GL()->EnableScissor(Viewport(Vector4i(x, y, width, height)));
		auto camera = std::make_shared<Camera>(*m_pCamera);
		camera->SetViewport(Vector4i(0, 0, width, height));
		const auto perspective = camera->GetPerspective();
		camera->SetPerspective(perspective.m_fov, width / static_cast<float>(height),
			perspective.m_near, perspective.m_far);
		CameraController controller(camera);
		controller.FitToBDB(box);
		if (m_animation) { controller.RotateAnimation(m_elapsed, box); }
		m_pResource->UpdateCamera(*camera);
		node->Update(m_elapsed);
		node->Draw(context);
	}
	m_pResource->GL()->DisableScissor();
	m_pResource->GL()->SetViewportFullWindow();
	m_pResource->UpdateCamera();
}

void STEPViewer::FitCamera()
{
	if (!m_pSTEPNode) { return; }
	const auto box = m_pSTEPNode->CalcCameraFitBox();
	if (std::isfinite(box.MaxLength()) && box.MaxLength() > 0.0f) {
		m_pCameraController->FitToBDB(box);
	}
}

void STEPViewer::Execute()
{
	DrawContext drawContext(m_pResource.get());
	Timer timer;
	timer.Reset();
	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		m_elapsed += timer.Tick();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (!m_galleryMode && m_pSTEPNode) {
			m_pSTEPNode->Update(m_elapsed);
			if (m_animation) {
				const auto box = m_pSTEPNode->CalcCameraFitBox();
				if (std::isfinite(box.MaxLength()) && box.MaxLength() > 0.0f) {
					m_pCameraController->RotateAnimation(m_elapsed, box);
				}
			}
		}
		m_pResource->UpdateCamera();
		if (m_galleryMode) {
			DrawGallery(drawContext);
		} else {
			if (m_pSTEPNode) { m_pSTEPNode->Draw(drawContext); }
			m_pDebugRoot->Draw(drawContext);
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		m_uiContext.SetViewport(m_windowSize);
		ShowUI();
		ImGui::Render();
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);
		glViewport(0, 0, width, height);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(m_window);
		glfwPollEvents();
		OUTPUT_GLERROR;
	}
}

void STEPViewer::ShowUI()
{
	if (ImGui::Begin("STEP Viewer")) {
		const char* folders[] = { "largeData", "ap224", "ap214", "ap209", "ap203e2", "ap203", "finish", "other" };
		if (ImGui::Combo("Folder", &m_folder, folders, 8)) { InitializeFiles(); }
		if (ImGui::Checkbox("Gallery Mode", &m_galleryMode)) { UpdateRenderData(); }
		ImGui::Checkbox("Animation", &m_animation);
		if (m_files.empty()) {
			ImGui::TextUnformatted("No STEP files in this folder list.");
		} else if (m_galleryMode) {
			const int maxPage = (static_cast<int>(m_files.size()) - 1) / GalleryPageSize;
			if (ImGui::SliderInt("Page", &m_galleryPage, 0, maxPage)) { UpdateRenderData(); }
			const int first = m_galleryPage * GalleryPageSize;
			const int end = std::min(first + GalleryPageSize, static_cast<int>(m_files.size()));
			for (int i = first; i < end; ++i) {
				ImGui::PushID(i);
				const String label = std::to_string(i - first + 1) + ": " + std::filesystem::path(m_files[i]).filename().string();
				if (ImGui::Selectable(label.c_str(), i == m_selectedFile)) {
					m_selectedFile = i;
					m_galleryMode = false;
					UpdateRenderData();
				}
				if (!m_nodes[i]) { ImGui::TextWrapped("%s", m_status[i].c_str()); }
				ImGui::PopID();
			}
		} else {
			bool changed = ImGui::SliderInt("File", &m_selectedFile, 0, static_cast<int>(m_files.size()) - 1);
			if (ImGui::Button("Previous")) { m_selectedFile = std::max(0, m_selectedFile - 1); changed = true; }
			ImGui::SameLine();
			if (ImGui::Button("Next")) { m_selectedFile = std::min(static_cast<int>(m_files.size()) - 1, m_selectedFile + 1); changed = true; }
			if (changed) { UpdateRenderData(); }
			if (ImGui::Button("Reload")) { m_pSTEPNode = LoadSTEP(m_selectedFile, true); FitCamera(); }
			ImGui::SameLine();
			if (ImGui::Button("Fit Camera")) { FitCamera(); }
			if (ImGui::Button("Clear Debug")) { m_uiContext.ClearDebugNode(); }
			ImGui::TextWrapped("%s", m_files[m_selectedFile].c_str());
			ImGui::TextWrapped("%s", m_status[m_selectedFile].c_str());
		}
		if (!m_galleryMode && m_pSTEPNode) {
			ImGui::Separator();
			m_pSTEPNode->ShowUI(m_uiContext);
		}
	}
	ImGui::End();
}

void STEPViewer::ProcessMouseEvent(const MouseInput& input)
{
	m_pMouse->ApplyMouseInput(input);
	if (ImGui::GetIO().WantCaptureMouse || m_galleryMode) { return; }
	EditContext context(m_pMouse.get(), m_pCamera.get());
	if (input.Event() == MOUSE_EVENT_WHEEL) {
		m_pCameraController->Wheel(context);
	} else if (input.Event() == MOUSE_EVENT_MOVE) {
		m_pCameraController->Move(context);
	}
}

void STEPViewer::ResizeEvent(int width, int height)
{
	glViewport(0, 0, width, height);
	m_windowSize = Vector2i(width, height);
	if (m_pCamera && width > 0 && height > 0) {
		m_pCamera->SetViewport(Vector4i(0, 0, width, height));
		m_pCameraController->SetAspect(static_cast<float>(width), static_cast<float>(height));
	}
	if (m_pResource) { m_pResource->GL()->SetWindowSize(m_windowSize); }
}

void STEPViewer::Finalize()
{
	m_uiContext.SetDebugNode(nullptr);
	m_pDebugRoot.reset();
	m_pSTEPNode.reset();
	m_nodes.clear();
	if (m_pResource) {
		m_pResource->Finalize();
		m_pResource.reset();
	}
	GLFWApp::Finalize();
}
}


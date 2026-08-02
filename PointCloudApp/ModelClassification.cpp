#include "ModelClassification.h"
#include "Mesh.h"
#include "GeometryLoader.h"

namespace KI
{

ModelClassification::ModelClassification()
{
}

ModelClassification::~ModelClassification()
{
}

void ModelClassification::Initialize()
{
	GLFWApp::Initialize();
}



namespace fs = std::filesystem;
Vector<ModelClassification::ModelNetData> ModelClassification::LoadModelNetTrain(const fs::path& root)
{
	Vector<ModelNetData> classes;

	// airplane, bathtub ...
	for (const auto& classDir : fs::directory_iterator(root)) {
		if (!classDir.is_directory())
			continue;

		fs::path trainDir = classDir.path() / "train";

		if (!fs::exists(trainDir))
			continue;

		ModelNetData data;
		data.className = classDir.path().filename().string();

		for (const auto& file : fs::directory_iterator(trainDir)) {
			if (!file.is_regular_file())
				continue;

			if (file.path().extension() != ".off")
				continue;

			data.files.emplace_back(file.path());
		}

		std::sort(data.files.begin(), data.files.end());

		classes.emplace_back(std::move(data));
	}

	std::sort(classes.begin(), classes.end(),
		[](const auto& a, const auto& b)
	{
		return a.className < b.className;
	});

	return classes;
}

void ModelClassification::LoadData(const String& offFile)
{
	auto mesh = std::unique_ptr<Mesh>(GeometryLoader::LoadOff(offFile));
	m_pMeshNode = std::make_shared<MeshNode>(offFile, *mesh);
	*m_pCamera = Camera::FitToBDB(*m_pCamera, m_pMeshNode->GetBoundBox());
}
void ModelClassification::Execute()
{
	m_data = LoadModelNetTrain("E:\\cgModel\\ModelNet40");
	LoadData(m_data[0].files[0].string());
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	
	m_pResource = std::make_unique<RenderResource>();
	m_pResource->Build();
	m_pResource->GL()->SetWindowSize(m_windowSize);
	m_pResource->GL()->EnablePolygonOffset(1.0f, 1.0f);
	m_pResource->GL()->SetLineWidth(5.0f);
	m_pResource->GL()->SetPointSize(5.0f);
	m_pResource->SetMainCamera(m_pCamera);
	DrawContext drawContext(m_pResource.get());
	UIContext ui;
	
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 400 core");





	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_pResource->UpdateCamera();
		m_pMeshNode->Draw(drawContext);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ui.SetViewport(m_windowSize);
		m_pMeshNode->ShowUI(ui);
		ShowUI();
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(m_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}
}

void ModelClassification::ShowUI()
{
	ImGui::Begin("ModelNet40 Browser");

	if (m_data.empty()) {
		ImGui::TextUnformatted("ModelNet40 data is empty.");
		ImGui::End();
		return;
	}

	ImGui::BeginChild("ClassListArea", ImVec2(220.0f, 400.0f), true);        // Å© ògÇ†ÇË

	ImGui::Text("Classes");
	ImGui::Separator();

	for (int i = 0; i < (int)m_data.size(); i++) {
		const bool selected = (m_ui.m_selectClassIndex == i);

		std::string label = m_data[i].className + " (" + std::to_string(m_data[i].files.size()) + ")";

		if (ImGui::Selectable(label.c_str(), selected)) {
			m_ui.m_selectClassIndex = i;
			m_ui.m_selectFileIndex = 0;
		}

		if (selected)
			ImGui::SetItemDefaultFocus();
	}

	ImGui::EndChild();
	ImGui::SameLine();


	ImGui::BeginGroup();
	const auto& selectedClass = m_data[m_ui.m_selectClassIndex];
	ImGui::Text("Selected class: %s", selectedClass.className.c_str());
	ImGui::Text("File count: %d", static_cast<int>(selectedClass.files.size()));
	ImGui::Separator();

	if (selectedClass.files.empty()) {
		ImGui::TextUnformatted("No OFF files.");
		ImGui::EndGroup();
		ImGui::End();
		return;
	}

	const int maxFileIndex = static_cast<int>(selectedClass.files.size()) - 1;

	m_ui.m_selectFileIndex = std::clamp(m_ui.m_selectFileIndex, 0, maxFileIndex);

	ImGui::SetNextItemWidth(400.0f);

	ImGui::SliderInt("File index", &m_ui.m_selectFileIndex, 0, maxFileIndex);

	if (ImGui::Button("Previous")) {
		m_ui.m_selectFileIndex = std::max(0, m_ui.m_selectFileIndex - 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("Next")) {
		m_ui.m_selectFileIndex = std::min(maxFileIndex, m_ui.m_selectFileIndex + 1);
	}

	const fs::path& selectedPath = selectedClass.files[m_ui.m_selectFileIndex];

	ImGui::Spacing();
	ImGui::Text("File: %s", selectedPath.filename().string().c_str());
	ImGui::TextWrapped("Path: %s", selectedPath.string().c_str());
	ImGui::Spacing();

	if (m_pMeshNode->GetName() != selectedPath) {
		LoadData(selectedPath.string());
	}

	ImGui::EndGroup();
	ImGui::End();
}

void ModelClassification::ProcessMouseEvent(const MouseInput& input)
{
	m_pMouse->ApplyMouseInput(input);
	EditContext context(m_pMouse.get(), m_pCamera.get());
	if (input.Event() == MOUSE_EVENT_WHEEL) {
		m_pCameraController->Wheel(context);
	} else if (input.Event() == MOUSE_EVENT_MOVE) {
		m_pCameraController->Move(context);
	}
}

void ModelClassification::ResizeEvent(int width, int height)
{
	glViewport(0, 0, width, height);
}

void ModelClassification::Finalize()
{
	m_pMeshNode.reset();
	glfwTerminate();
}
}
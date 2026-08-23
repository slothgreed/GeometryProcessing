#include "MeshViewer.h"
#include "Mesh.h"
#include "GeometryLoader.h"
#include "FileUtility.h"
#include "PrimitiveNode.h"
#include "PointCloud.h"
#include "PointCloudNode.h"
#include "Profiler.h"
#include "PointCloudIO.h"
#include "HalfEdgeStruct.h"
#include "HalfEdgeNode.h"
namespace KI
{

MeshViewer::MeshViewer()
	:m_loadType(DIFFUSION_AI)
{
}

MeshViewer::~MeshViewer()
{
}

void MeshViewer::Initialize()
{
	GLFWApp::Initialize();
}


Shared<RenderNode> MeshViewer::LoadModelNetData(const AIDataFolder& folder, int fileIndex)
{
	if (folder.datas.size() <= fileIndex) { return nullptr; }
	const auto& data = folder.datas[fileIndex];
	const auto& filePath = data.filePath; 
	if (FileUtility::GetExtension(filePath.string()) == ".off") {
		auto mesh = std::unique_ptr<Mesh>(GeometryLoader::LoadOff(filePath.string()));
		auto pRenderNode = std::make_shared<MeshNode>(filePath.string(), *mesh);
		pRenderNode->SetVisibleVertex(false);
		pRenderNode->SetVisibleTriangle(true);
		return pRenderNode;
	} else if (FileUtility::GetExtension(filePath.string()) == PointCloudIO::binExt) {
		auto pPointCloud = std::unique_ptr<PointCloud>(PointCloudIO::Load(filePath.string()));
		auto pos = pPointCloud->Position();
		auto mesh = std::make_unique<Mesh>(std::move(pos), Mesh::DrawType::Triangles);
		auto pRenderNode = std::make_shared<MeshNode>(filePath.string(), *mesh);
		pRenderNode->SetVisibleVertex(false);
		pRenderNode->SetVisibleTriangle(false);
		return pRenderNode;
	}

	return nullptr;
}
Shared<RenderNode> MeshViewer::LoadMeshsegData(const AIDataFolder& folder, int fileIndex, int segIndex)
{
	if (folder.datas.size() <= fileIndex) { return nullptr; }
	const auto& data = folder.datas[fileIndex];
	if (data.segFile.size() <= segIndex) { return nullptr; }

	const auto& filePath = data.filePath;
	if (FileUtility::GetExtension(filePath.string()) == ".off") {
		auto mesh = std::unique_ptr<Mesh>(GeometryLoader::LoadOff(filePath.string()));
		auto halfEdge = std::make_shared<HalfEdgeStruct>(HalfEdgeStruct::Create(*mesh));
		auto pRenderNode = std::make_shared<HalfEdgeNode>(filePath.string(), std::move(halfEdge));
		return pRenderNode;
		auto pPrimitive = std::make_shared<Primitive>();
		pPrimitive->SetPosition(mesh->GetPoints());
		const auto& segFile = data.segFile[segIndex];
		auto segColor = AIDataGenerator::LoadMeshsegBenchmarkSeg(segFile);
		pPrimitive->SetColor(std::move(segColor));
		pPrimitive->SetType(GL_TRIANGLES);
		return std::make_shared<PrimitiveNode>(filePath.string(), pPrimitive);
	}

	return nullptr;
}



Shared<RenderNode> MeshViewer::LoadMeshCNNBunny()
{
	struct FaceSerializer
	{
		float centerX;
		float centerY;
		float centerZ;
		float normalX;
		float normalY;
		float normalZ;
		float area;
		float edgeLength0;
		float edgeLength1;
		float edgeLength2;
		float angle0;
		float angle1;
		float angle2;
		int32_t neighbor0;
		int32_t neighbor1;
		int32_t neighbor2;
	};
	Vector<FaceSerializer> faces;

	std::string path = "E:\\cgModel\\MeshsegBenchmark-1.0\\data\\predict\\bunny.meshcnn";
	std::ifstream ifs(path, std::ios::binary);
	uint32_t faceCount = 0;
	ifs.read(reinterpret_cast<char*>(&faceCount), sizeof(faceCount));
	faces.resize(faceCount);
	ifs.read(reinterpret_cast<char*>(faces.data()), sizeof(FaceSerializer) * faceCount);
	Vector<Vector3> points(faceCount);
	for (size_t i = 0; i < points.size(); i++) {
		points[i] = Vector3(faces[i].centerX, faces[i].centerY, faces[i].centerZ);
	}

	auto pPrimitive = std::make_shared<Primitive>();
	pPrimitive->SetPosition(points);
	auto segColor = AIDataGenerator::LoadSegBinary("E:\\cgModel\\MeshsegBenchmark-1.0\\data\\predict\\bunny.seg");
	pPrimitive->SetColor(std::move(segColor));
	pPrimitive->SetType(GL_POINTS);
	return std::make_shared<PrimitiveNode>(path, pPrimitive);
}

Shared<RenderNode> MeshViewer::LoadPointCloudBynny()
{
	const String path = "E:\\cgModel\\Diffusion\\bunny4000.xyz.ki_bin";
	Shared<PointCloud> pPointCloud(PointCloudIO::Load(path));
	pPointCloud->ClearColor();
	//PointCloudIO::OutputBinary(pPointCloud.get(), pathBin.c_str(), false, false);

	return std::make_shared<PointCloudNode>("Diffusion Bunny", pPointCloud);
}

void MeshViewer::Execute()
{
	if (m_loadType == MODEL_CLASSIFICATION) {
		m_folder = AIDataGenerator::LoadModelNetTrain("E:\\cgModel\\ModelNet40");
	} else if (m_loadType == MESH_SEGMENTATION || m_loadType == LOAD_BUNNY) {
		m_folder = AIDataGenerator::LoadMeshsegBenchmark("E:\\cgModel\\MeshsegBenchmark-1.0\\data");
	}
	UpdateMeshCategory();
	UpdateRenderData();

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



	Timer timer;
	m_timerDiff = 0;


	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		timer.Start();
		if (m_pRenderNode) {
			m_pRenderNode->Update(m_timerDiff);
		}
		if (m_ui.m_garallyMode) {
			DrawGaralley(drawContext);
		} else {
			m_pResource->UpdateCamera();
			if (m_pRenderNode) {
				m_pRenderNode->Draw(drawContext);
			}
			if (m_ui.animation) {
				m_pCameraController->RotateAnimation(m_timerDiff, m_pRenderNode->CalcCameraFitBox());
			}
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ui.SetViewport(m_windowSize);
		if (m_pRenderNode) {
			m_pRenderNode->ShowUI(ui);
		}


		m_timerDiff += timer.Stop() * 10;
		if (m_timerDiff > 100000.0) { m_timerDiff = 0.0f; }

		ShowUI();
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(m_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(m_window);

		glfwPollEvents();
		OUTPUT_GLERROR;
	}
}

void MeshViewer::UpdateRenderData()
{
	if (m_ui.m_garallyMode) { UpdateGaralley(); return; }
	if (m_loadType == MODEL_CLASSIFICATION) {
		m_pRenderNode = LoadModelNetData(m_folder[m_ui.m_selectCategoryIndex], m_ui.m_selectFileIndex);
	} else if(m_loadType == MESH_SEGMENTATION) {
		m_pRenderNode = LoadMeshsegData(m_folder[m_ui.m_selectCategoryIndex], m_ui.m_selectFileIndex, m_ui.m_selectSegIndex);
	} else if (m_loadType == LOAD_BUNNY) {
		m_pRenderNode = LoadMeshCNNBunny();
	} else if (m_loadType == DIFFUSION_AI) {
		m_pRenderNode = LoadPointCloudBynny();
	}
	if (m_pRenderNode) {
		m_pCameraController->FitToBDB(m_pRenderNode->GetBoundBox());
	}
}
bool MeshViewer::ShowSliderUI(const String& name, int minValue, int maxValue, int* ret)
{
	bool modify = false;
	ImGui::SetNextItemWidth(400.0f);
	if (ImGui::SliderInt(name.c_str(), ret, minValue, maxValue)) {
		modify = true;
	}

	if (ImGui::Button((name + " Previous").c_str())) {
		*ret = std::max(0, *ret - 1);
		modify = true;
	}

	ImGui::SameLine();

	if (ImGui::Button((name + " Next").c_str())) {
		*ret = std::min(maxValue, *ret + 1);
		modify = true;
	}
	return modify;
}
void MeshViewer::ShowUI()
{
	if (m_folder.empty()) { return; }
	ImGui::Begin("ModelNet40 Browser");
	ImGui::Checkbox("Animation", &m_ui.animation);
	if (ImGui::Checkbox("Garalley Mode", &m_ui.m_garallyMode)) {
		UpdateRenderData();
	}

	if(m_ui.m_garallyMode) {
		ShowGaralleyUI();
		ImGui::End();
		return;
	}
	

	bool modify = false;
	const int maxClassIndex = static_cast<int>(m_folder.size()) - 1;
	if (ShowSliderUI("Category", 0, maxClassIndex, &m_ui.m_selectCategoryIndex)) {
		m_ui.m_selectFileIndex = 0;
		m_ui.m_selectSegIndex = 0;
		modify = true;
	}

	const auto& selectedCategory = m_folder[m_ui.m_selectCategoryIndex];
	if (!selectedCategory.datas.empty()) {
		const int maxFileIndex = static_cast<int>(selectedCategory.datas.size()) - 1;
		if (ShowSliderUI("File", 0, maxFileIndex, &m_ui.m_selectFileIndex)) {
			m_ui.m_selectSegIndex = 0;
			modify = true;
		}
	}

	if (selectedCategory.datas.empty()) {
		ImGui::End();
		return;
	}

	const auto& selectedData = selectedCategory.datas[m_ui.m_selectFileIndex];
	if (!selectedData.segFile.empty()) {
		const int maxSegIndex = static_cast<int>(selectedData.segFile.size()) - 1;
		if (ShowSliderUI("Seg", 0, maxSegIndex, &m_ui.m_selectSegIndex)) {
			modify = true;
		}
	}

	if (modify) {
		UpdateRenderData();
	}

	ImGui::Separator();
	ImGui::Text("Category : %s", selectedCategory.className.c_str());
	ImGui::Text("File     : %s", selectedData.filePath.string().c_str());
	if (!selectedData.segFile.empty()) {
		const auto& selectedSegPath = selectedData.segFile[m_ui.m_selectSegIndex];
		ImGui::Text("Seg      : %s", selectedSegPath.string().c_str());
	}
	ImGui::End();

}

void MeshViewer::ProcessMouseEvent(const MouseInput& input)
{
	m_pMouse->ApplyMouseInput(input);
	EditContext context(m_pMouse.get(), m_pCamera.get());
	if (input.Event() == MOUSE_EVENT_WHEEL) {
		m_pCameraController->Wheel(context);
	} else if (input.Event() == MOUSE_EVENT_MOVE) {
		m_pCameraController->Move(context);
	}
}

void MeshViewer::ResizeEvent(int width, int height)
{
	glViewport(0, 0, width, height);
	m_windowSize = Vector2i(width, height);
	m_pResource->GL()->SetWindowSize(m_windowSize);
}

void MeshViewer::Finalize()
{
	m_pRenderNode.reset();
	glfwTerminate();
}

void MeshViewer::UpdateMeshCategory()
{
	m_folderInfo.maxFileNum = 0;
	m_folderInfo.maxSegNum = 0;
	for (int i = 0; i < m_folder.size(); i++) {
		m_folderInfo.maxFileNum = std::max((int)m_folder[i].datas.size(), m_folderInfo.maxFileNum);
		for(int j = 0; j < m_folder[i].datas.size(); j++) {
			m_folderInfo.maxSegNum = std::max((int)m_folder[i].datas[j].segFile.size(), m_folderInfo.maxSegNum);
		}
	}
}

int MeshViewer::GetGaralleyPageNum() const
{
	if (m_loadType == MODEL_CLASSIFICATION) {
		return m_folderInfo.maxFileNum - 1;
	} else if (m_loadType == MESH_SEGMENTATION) {
		return (m_folder.size() - 1) / m_garalleyData.maxPageNum;
	}

	return 0;
}

void MeshViewer::UpdateGaralley()
{
	m_pGaralleyNode.resize(m_garalleyData.maxPageNum);
	std::fill(m_pGaralleyNode.begin(), m_pGaralleyNode.end(), nullptr);
	for (int i = 0; i < m_garalleyData.maxPageNum; i++) {
		if (m_loadType == MODEL_CLASSIFICATION) {
			m_pGaralleyNode[i] = LoadModelNetData(m_folder[i], m_garalleyData.selectPage);
		} else if(m_loadType == MESH_SEGMENTATION) {
			int index = m_garalleyData.selectPage * m_garalleyData.maxPageNum + i;
			m_pGaralleyNode[i] = LoadMeshsegData(m_folder[index], 0, m_garalleyData.segIndex);
		}
	}
}
void MeshViewer::DrawGaralley(const DrawContext& context)
{
	auto windowSize = context.pResource->GL()->GetWindowSize();
	int w = windowSize.x / m_garalleyData.width;
	int h = windowSize.y / m_garalleyData.height;
	for (int i = 0; i < m_garalleyData.width; i++) {
		for (int j = 0; j < m_garalleyData.height; j++) {
			int index = i + j * m_garalleyData.width;
			if (m_pGaralleyNode[index] == nullptr) { continue; }
			context.pResource->GL()->SetViewport(Viewport(Vector4i(i * w, j * h, w, h)));
			m_pCameraController->FitToBDB(m_pGaralleyNode[index]->GetBoundBox());
			if (m_ui.animation) {
				m_pCameraController->RotateAnimation(m_timerDiff, m_pGaralleyNode[index]->CalcCameraFitBox());
			}

			m_pResource->UpdateCamera();
			m_pGaralleyNode[index]->Draw(context);
		}
	}
	context.pResource->GL()->SetViewportFullWindow();
}
void MeshViewer::ShowGaralleyUI()
{
	bool modify = false;
	if (ImGui::SliderInt("Garalley Page", &m_garalleyData.selectPage, 0, GetGaralleyPageNum())) {
		modify = true;
	}
	if (ImGui::SliderInt("Seg Index", &m_garalleyData.segIndex, 0, m_folderInfo.maxSegNum)) {
		modify = true;
	}
	if (modify) {
		UpdateGaralley();
	}
}
}

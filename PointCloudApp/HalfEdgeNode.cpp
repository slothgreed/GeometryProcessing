#include "HalfEdgeNode.h"
#include "SimpleShader.h"
#include "Utility.h"
#include "MeshletGenerator.h"
#include "ShapeDiameterFunction.h"
#include "SignedDistanceField.h"
#include "Voxelizer.h"
#include "Primitive.h"
#include "BVH.h"
#include "GeometryUtility.h"
#include "PrimitiveNode.h"
#include "HalfEdgeController.h"
namespace KI
{

HalfEdgeNode::HalfEdgeNode(const String& name, const Shared<HalfEdgeStruct>& pStruct)
	: RenderNode(name)
	, m_pHalfEdge(pStruct)
	, m_pBVH(nullptr)
	, m_pVoxelizer(nullptr)
	, m_pSignedDistanceField(nullptr)
	, m_pShapeDiameterFunction(nullptr)
	, m_pController(new HalfEdgeController(this))
{

	m_updateData.set();
	BuildGLBuffer();
	SetBoundBox(pStruct->CreateBDB());

	m_pickIds.face.begin = 0;
	m_pickIds.face.num = m_pHalfEdge->GetFaceNum();

	m_pickIds.edge.begin = m_pickIds.face.num;
	m_pickIds.edge.num = m_pHalfEdge->GetEdgeNum();

	m_pickIds.vertex.begin = m_pickIds.face.num + m_pickIds.edge.num;
	m_pickIds.vertex.num = m_pHalfEdge->GetVertexNum();
}

HalfEdgeNode::~HalfEdgeNode()
{
	RELEASE_INSTANCE(m_pController);
	RELEASE_INSTANCE(m_pShapeDiameterFunction);
	RELEASE_INSTANCE(m_pVoxelizer);
	RELEASE_INSTANCE(m_pBVH);

}

const HalfEdgeNode::HalfEdgeParts* HalfEdgeNode::HalfEdgeParts::Cast(const RenderParts* pParts)
{
	assert(dynamic_cast<const HalfEdgeNode::HalfEdgeParts*>(pParts));
	return static_cast<const HalfEdgeNode::HalfEdgeParts*>(pParts);
}

String HalfEdgeNode::HalfEdgeParts::ToString()
{
	std::stringstream ss;
	if (type == Type::Face) {
		ss << "Face : " << parts;
	} else if (type == Type::Edge) {
		ss << "Edge : " << parts;
	} else if (type == Type::Vertex) {
		ss << "Vertex : " << parts;
	}
	return ss.str();
}
void HalfEdgeNode::BuildGLBuffer()
{
	if (!m_updateData.any()) { return; }
	if (m_updateData[UPDATE_VERTEX]) {
		if (!m_gpu.position) {
			m_gpu.position = std::make_unique<GLBuffer>();
			m_gpu.position->Create(m_pHalfEdge->GetVertex());
		} else {
			m_gpu.position->BufferSubData(0, m_pHalfEdge->GetVertex());
			return;
		}
	}
	m_gpu.normal = std::make_unique<GLBuffer>();
	m_gpu.normal->Create(m_pHalfEdge->GetNormal());
	m_gpu.faceIndexBuffer = std::make_unique<GLBuffer>();
	m_gpu.faceIndexBuffer->Create(m_pHalfEdge->CreateIndexBufferData());

	m_gpu.vertexColor = std::make_unique<GLBuffer>();
	m_gpu.vertexDir1 = std::make_unique<GLBuffer>();
	m_gpu.vertexDir2 = std::make_unique<GLBuffer>();
}

void HalfEdgeNode::ShowNormal(const DrawContext& context)
{
	auto pVertexVector = context.pResource->GetShaderTable()->GetVertexVectorShader();
	pVertexVector->Use();
	pVertexVector->SetColor(Vector4(1, 0, 0, 1));
	pVertexVector->SetLength(m_ui.normalLength);
	pVertexVector->SetModel(GetMatrix());
	pVertexVector->SetPosition(m_gpu.position.get());
	pVertexVector->SetVector(m_gpu.normal.get());
	pVertexVector->DrawArray(GL_POINTS, m_gpu.position->Num());
}
void HalfEdgeNode::BuildEdge()
{
	if (m_gpu.edgeIndexBuffer) { return; }
	m_gpu.edgeIndexBuffer = std::make_unique<GLBuffer>();
	m_gpu.edgeIndexBuffer->Create(m_pHalfEdge->CreateEdgeIndexBufferData());
}

void HalfEdgeNode::BuildMorton()
{
	if (m_morton.gpuLine) { return; }
	m_morton.data.Create(m_pHalfEdge->GetVertex(),m_pHalfEdge->CreateIndexBufferData(),GetBoundBox());
	
	Vector<Vector3> position(m_morton.data.Get().size());
	Vector<Vector3> color(m_morton.data.Get().size());
	for (auto i = 0; i < m_morton.data.Get().size(); i++) {
		position[i] = m_pHalfEdge->CalcGravity(m_morton.data.Get()[i].triangleIndex);
		color[i] = MortonCode::ToColor(m_morton.data.Get()[i].morton);
	}

	m_morton.gpuLine = std::make_unique<GLBuffer>();
	m_morton.gpuLine->Create(position);

	m_morton.gpuColor = std::make_unique<GLBuffer>();
	m_morton.gpuColor->Create(color);
}

BVH* HalfEdgeNode::GetBVH()
{
	if (m_pBVH == nullptr) {
		BuildBVH();
	}
	return m_pBVH;
}
GLBuffer* HalfEdgeNode::GetBVHGpu()
{
	if (m_pBVH == nullptr) {
		BuildBVH();
	}

	return m_gpu.bvh.get();
}
void HalfEdgeNode::BuildBVH()
{
	if (m_pBVH) { return; }
	m_pBVH = new BVH(this);
	BuildMorton();
	m_pBVH->Execute();
	m_gpu.bvh = std::make_unique<GLBuffer>();
	m_gpu.bvh->Create<KI::BVH::Node>(m_pBVH->GetNode());
}


void HalfEdgeNode::DrawNode(const DrawContext& context)
{
	BuildGLBuffer();
	if (!m_ui.visible) { return; }
	auto pResource = context.pResource;
	auto& pFaceShader = pResource->GetShaderTable()->GetFaceShader();
	context.pResource->GetRenderTarget()->Bind();
	context.pResource->GL()->PushRenderTarget(context.pResource->GetRenderTarget(), pFaceShader->GetDrawTargetNum());
	if (!m_ui.visibleMesh) {
		context.pResource->GL()->ColorMask(false);
	}
	pFaceShader->Use();
	pFaceShader->SetCamera(pResource->GetCameraBuffer());
	pFaceShader->SetLight(context.pResource->GetLightBuffer());
	pFaceShader->SetPBRResource(context.pResource->GetPBR());
	pFaceShader->SetPosition(m_gpu.position.get());
	pFaceShader->SetNormal(m_gpu.normal.get());
	pFaceShader->SetColor(Vector3(0.7f, 0.7f, 1.0f));
	pFaceShader->SetModel(GetMatrix());
	pFaceShader->DrawElement(GL_TRIANGLES, m_gpu.faceIndexBuffer.get());
	context.pResource->GL()->PopRenderTarget();
	if (!m_ui.visibleMesh) {
		context.pResource->GL()->ColorMask(true);
	}

	if (m_ui.visibleEdge || m_ui.visibleVertex) {
		auto& pSimpleShader = pResource->GetShaderTable()->GetSimpleShader();
		pSimpleShader->Use();
		pSimpleShader->SetPosition(m_gpu.position.get());
		pSimpleShader->SetCamera(pResource->GetCameraBuffer());
		pSimpleShader->SetModel(GetMatrix());
		pSimpleShader->SetColor(Vector3(0.7f, 0.7f, 1.0f));
		if (m_ui.visibleEdge) {
			pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
			pSimpleShader->DrawElement(GL_LINES, m_gpu.edgeIndexBuffer.get());
		}

		if (m_ui.visibleVertex) {

			if ((HalfEdgeStruct::VertexValue)m_ui.vertexValue == HalfEdgeStruct::VertexValue::None) {
				pSimpleShader->SetPosition(m_gpu.position.get());
				pSimpleShader->SetCamera(pResource->GetCameraBuffer());
				pSimpleShader->SetModel(GetMatrix());
				pSimpleShader->SetColor(Vector3(1.0f, 0.0f, 0.0f));
				pSimpleShader->DrawArray(GL_POINTS, m_pHalfEdge->GetVertexNum());
			} else {
				auto pVertexColor = pResource->GetShaderTable()->GetVertexColorShader();
				pVertexColor->Use();
				pVertexColor->SetPosition(m_gpu.position.get());
				pVertexColor->SetCamera(pResource->GetCameraBuffer());
				pVertexColor->SetModel(GetMatrix());
				pVertexColor->SetColor(m_gpu.vertexColor.get());
				pVertexColor->DrawArray(GL_POINTS, m_pHalfEdge->GetVertexNum());
			}

			if ((HalfEdgeStruct::VertexDirection)m_ui.vertexDirection != HalfEdgeStruct::VertexDirection::None) {
				auto pVertexVector = context.pResource->GetShaderTable()->GetVertexVectorShader();
				pVertexVector->Use();
				pVertexVector->SetLength(m_ui.normalLength);
				pVertexVector->SetModel(GetMatrix());
				pVertexVector->SetPosition(m_gpu.position.get());
				pVertexVector->SetColor(Vector4(0, 1, 0, 1));
				pVertexVector->SetVector(m_gpu.vertexDir1.get());
				pVertexVector->DrawArray(GL_POINTS, m_gpu.position->Num());
				pVertexVector->SetColor(Vector4(0, 0, 1, 1));
				pVertexVector->SetVector(m_gpu.vertexDir2.get());
				pVertexVector->DrawArray(GL_POINTS, m_gpu.position->Num());
			}
		}
	}

	if (m_ui.visibleMorton) {
		auto pVertexColor = pResource->GetShaderTable()->GetVertexColorShader();
		pVertexColor->Use();
		pVertexColor->SetPosition(m_morton.gpuLine.get());
		pVertexColor->SetCamera(pResource->GetCameraBuffer());
		pVertexColor->SetModel(GetMatrix());
		pVertexColor->SetColor(m_morton.gpuColor.get());
		pVertexColor->DrawArray(GL_LINE_STRIP, 0, m_pHalfEdge->GetFaceNum());
	}

	if (m_ui.visibleNormal) {
		ShowNormal(context);
	}

	if (m_ui.voxel.visible) {
		if (m_pVoxelizer) {
			m_pVoxelizer->Draw(m_gpu.position.get(), m_gpu.faceIndexBuffer.get(), pResource->GetCameraBuffer()->Handle());
		}
	}


	if (m_ui.meshlet.visible && m_meshletGpu.shader) {
		m_meshletProfiler->BeginQuery();
		m_meshletGpu.shader->Use();
		m_meshletGpu.taskNum->SetData(0);
		m_meshletGpu.shader->SetPosition(m_meshletGpu.position.get());
		m_meshletGpu.shader->SetMeshlet(m_meshletGpu.cluster.get());
		m_meshletGpu.shader->SetTaskToMeshNum(m_meshletGpu.taskNum.get());
		m_meshletGpu.shader->SetIndex(m_meshletGpu.index.get());
		m_meshletGpu.shader->SetCamera(pResource->GetCameraBuffer());
		m_meshletGpu.shader->SetModel(GetMatrix());
		m_meshletGpu.shader->SetNormalMatrix(GetNormalMatrix());
		m_meshletGpu.shader->SetMeshletNum(m_meshletGpu.cluster->Num());
		m_meshletGpu.shader->SetCullSize(m_ui.meshlet.cullSize);
		//for (int x = -10; x < 10; x++) 
		//for (int y = -10; y < 10; y++) 
		//for (int z = -10; z < 10; z++)
		{
			m_meshletGpu.shader->DrawWithAutoTask(0, m_meshletGpu.cluster->Num());
		}
		/*
		std::vector<unsigned int> taskNum(m_meshletGpu.taskNum->Num());
		m_meshletGpu.taskNum->GetBufferData(taskNum);
		auto totalSize = 0;
		for (int i = 0; i < taskNum.size(); i++) {
			totalSize+=taskNum[i];
		}
		printf("TaskNum : %d\n", totalSize);
		*/

		m_meshletGpu.shader->BarrierSSBO();
		m_meshletProfiler->EndQuery();
	}
	
}

void HalfEdgeNode::PickNode(const PickContext& context)
{
	if (!m_ui.visible) { return; }

	auto pResource = context.pResource;
	auto& pPickShader = pResource->GetShaderTable()->GetPointPickByPrimitive();
	pPickShader->Use();
	pPickShader->SetPosition(m_gpu.position.get());
	pPickShader->SetCamera(pResource->GetCameraBuffer());
	pPickShader->SetModel(GetMatrix());
	pPickShader->SetPickOffset(m_pickIds.face.begin);
	if (m_ui.visibleMesh) {
		pPickShader->DrawElement(GL_TRIANGLES, m_gpu.faceIndexBuffer.get());
	}

	if (m_ui.visibleEdge) {
		pPickShader->SetPickOffset(m_pickIds.edge.begin);
		pPickShader->DrawElement(GL_LINES, m_gpu.edgeIndexBuffer.get());
	}

	if (m_ui.visibleVertex) {
		pPickShader->SetPickOffset(m_pickIds.vertex.begin);
		pPickShader->DrawArray(GL_POINTS, m_pHalfEdge->GetVertexNum());
	}
}

void HalfEdgeNode::UpdateVertex()
{
	m_updateData[UPDATE_VERTEX] = true;
}
bool HalfEdgeNode::CollectPickedNode(PickResult& result)
{
	Shared<HalfEdgeParts> pPick = nullptr;
	if (m_pickIds.face.Inner(result.id)) {
		int faceId = result.id - m_pickIds.face.begin;
		pPick = std::make_shared<HalfEdgeParts>(HalfEdgeParts::Type::Face, faceId);
	} else if (m_pickIds.edge.Inner(result.id)) {
		int edgeId = result.id - m_pickIds.edge.begin;
		pPick = std::make_shared<HalfEdgeParts>(HalfEdgeParts::Type::Edge, edgeId);
	} else if (m_pickIds.vertex.Inner(result.id)) {
		int vertexId = result.id - m_pickIds.vertex.begin;
		pPick = std::make_shared<HalfEdgeParts>(HalfEdgeParts::Type::Vertex, vertexId);
		if (m_ui.visibleVertex) {
			m_pEditVertex = pPick;
		} else {
			m_pEditVertex = nullptr;
		}
		if (pPick) {
			result.pResult[this] = pPick;
		} else {
			result.pResult.erase(this);
		}
		return true;
	}
	if (pPick) {
		result.pResult[this] = pPick;
	} else {
		result.pResult.erase(this);
	}
	return pPick != nullptr;
}

void HalfEdgeNode::ProcessMouseEvent(const PickContext& context)
{
	if (context.pMouse->Event() == MOUSE_EVENT_DOWN) {
		if (!m_pEditVertex || 
			!m_pEditVertex->IsVertex()) {
			m_pController->SetTarget(nullptr);
		} else {
			m_pController->SetTarget(m_pEditVertex);
		}
	} else if (context.pMouse->Event() == MOUSE_EVENT_UP) {
		m_pEditVertex = nullptr;
		m_pController->SetTarget(nullptr);
	}

}
void HalfEdgeNode::DrawPartsNode(const DrawContext& context, const RenderParts& parts)
{
	auto pResource = context.pResource;
	context.pResource->GL()->DisableDepth();
	auto drawParts = HalfEdgeParts::Cast(&parts);
	auto pSimpleShader = pResource->GetShaderTable()->GetSimpleShader();
	pSimpleShader->Use();
	pSimpleShader->SetPosition(m_gpu.position.get());
	pSimpleShader->SetCamera(pResource->GetCameraBuffer());
	pSimpleShader->SetModel(GetMatrix());
	pSimpleShader->SetColor(Vector3(1.0f, 0.7f, 1.0f));
	if (drawParts->IsFace()) {
		pSimpleShader->DrawElement(GL_TRIANGLES, m_gpu.faceIndexBuffer.get(), 3, drawParts->parts * 3);
	} else if (drawParts->IsEdge()) {
		pSimpleShader->DrawElement(GL_LINES, m_gpu.edgeIndexBuffer.get(), 2, drawParts->parts * 2);
	} else if (drawParts->IsVertex()) {
		pSimpleShader->DrawArray(GL_POINTS, drawParts->parts, 1);
	}
	context.pResource->GL()->EnableDepth();

}


void HalfEdgeNode::ShowUI(UIContext& ui)
{
	ImGui::Checkbox("Visible", &m_ui.visible);
	ImGui::Text("VertexNum%d, EdgeNum%d,TriangleNum %d",
		m_pHalfEdge->GetVertexNum(),
		m_pHalfEdge->GetEdgeNum(),
		m_pHalfEdge->GetFaceNum());

	ImGui::Checkbox("ShowMesh", &m_ui.visibleMesh);
	if (ImGui::Checkbox("ShowEdge", &m_ui.visibleEdge)) {
		if (m_ui.visibleEdge) {
			BuildEdge();
		}
	}

	ImGui::Checkbox("ShowVertex", &m_ui.visibleVertex);
	if (m_ui.visibleVertex) {
		auto param = (HalfEdgeStruct::VertexValue)m_ui.vertexValue;
		if (ImGui::Combo("VertexColor", &m_ui.vertexValue, HalfEdgeStruct::GetVertexValueString(), static_cast<int>(HalfEdgeStruct::VertexValue::Num))) {
			param = (HalfEdgeStruct::VertexValue)m_ui.vertexValue;
			if (param == HalfEdgeStruct::VertexValue::HeatValue) {
				m_pHalfEdge->CreateHeatMethod(m_ui.heatMethod.timeStep, 0);
				m_vertexParameter = Parameter(HalfEdgeStruct::ToString(param), m_pHalfEdge->GetHeatValue());
			} else if (param == HalfEdgeStruct::VertexValue::VertexArea) {
				m_pHalfEdge->CreateVertexArea();
				m_vertexParameter = Parameter(HalfEdgeStruct::ToString(param), m_pHalfEdge->GetVertexArea());
			} else if (param == HalfEdgeStruct::VertexValue::SDF) {
				if (m_pShapeDiameterFunction == nullptr) m_pShapeDiameterFunction = new ShapeDiameterFunction(this);
				m_pShapeDiameterFunction->Execute();
				m_vertexParameter = Parameter(HalfEdgeStruct::ToString(param), m_pShapeDiameterFunction->GetResult());
			} else if (param == HalfEdgeStruct::VertexValue::MinCurvature) {
				m_pHalfEdge->CreateDirectionField();
				m_vertexParameter = Parameter(HalfEdgeStruct::ToString(param), m_pHalfEdge->GetMinCurvature());
			} else if (param == HalfEdgeStruct::VertexValue::MaxCurvature) {
				m_pHalfEdge->CreateDirectionField();
				m_vertexParameter = Parameter(HalfEdgeStruct::ToString(param), m_pHalfEdge->GetMaxCurvature());
			} else {
				m_vertexParameter = Parameter();
			}

			if (m_vertexParameter.IsActive()) {
				m_gpu.vertexColor->Create(m_vertexParameter.CreatePseudoColor());
			}
		}

		auto paramDir = (HalfEdgeStruct::VertexDirection)m_ui.vertexDirection;
		if (ImGui::Combo("VertexDirection", &m_ui.vertexDirection, HalfEdgeStruct::GetVertexDirectionString(), static_cast<int>(HalfEdgeStruct::VertexDirection::Num))) {
			auto param = (HalfEdgeStruct::VertexDirection)m_ui.vertexDirection;
			if (param != HalfEdgeStruct::VertexDirection::None) {
				m_pHalfEdge->CreateDirectionField();
				m_gpu.vertexDir1->Create(m_pHalfEdge->GetMinDirection());
				m_gpu.vertexDir2->Create(m_pHalfEdge->GetMaxDirection());
			}
		}

		if (param == HalfEdgeStruct::VertexValue::HeatValue) {
			if (ImGui::SliderFloat("HeatTimeStep", &m_ui.heatMethod.timeStep, 0.0f, 10.0f)) {
				m_pHalfEdge->CreateHeatMethod(m_ui.heatMethod.timeStep, 0);
				m_vertexParameter = Parameter(HalfEdgeStruct::ToString(param), m_pHalfEdge->GetHeatValue());
				m_gpu.vertexColor->Create(m_vertexParameter.CreatePseudoColor());
			}
		}
		if (param != HalfEdgeStruct::VertexValue::None) {
			ShowUIParameter(m_vertexParameter, ui);
		}
	}

	ImGui::Checkbox("ShowNormal", &m_ui.visibleNormal);
	if (m_ui.visibleNormal) {
		ImGui::SliderFloat("NormalLength", &m_ui.normalLength, 0.0f, 1.0f);
	}

	ImGui::Checkbox("VisibleMeshlet", &m_ui.meshlet.visible);
	if (m_ui.meshlet.visible) {
		if (ImGui::SliderInt("MeshLetGenerate", &m_ui.meshlet.level, 0, 7)) {
			if (!m_meshletProfiler) { m_meshletProfiler = std::make_unique<MeshletProfiler>(); }
			auto meshlet = MeshletGenerator::Execute(*m_pHalfEdge.get(), m_ui.meshlet.level);
			m_meshletGpu.cluster = std::make_unique<GLBuffer>();
			m_meshletGpu.cluster->Create<Meshlet::Cluster>(meshlet.cluster);

			m_meshletGpu.index = std::make_unique<GLBuffer>();
			m_meshletGpu.index->Create<int>(meshlet.index);
			m_meshletGpu.position = std::make_unique<GLBuffer>();
			m_meshletGpu.position->Create(TypeConverter::Convert4f(m_pHalfEdge->GetVertex()));
			m_meshletGpu.taskNum = std::make_unique<GLBuffer>();
			m_meshletGpu.taskNum->Create(meshlet.cluster.size(), sizeof(unsigned int));
			m_meshletGpu.shader = std::make_unique<MeshletShader>();
			m_meshletGpu.shader->Build();
		}
		ImGui::SliderInt("MeshLetCullSize", &m_ui.meshlet.cullSize, 0, 100);
		if (m_meshletProfiler) {
			m_meshletProfiler->ShowUI();
		}
	}

	if (ImGui::Checkbox("ShowBVH", &m_ui.visibleBVH)) {
		if (m_ui.visibleBVH) {
			BuildBVH();
		} else {
			if (m_pBVH) {
				m_pBVH->DeleteUINode();
			}
		}
	} 

	if (m_ui.visibleBVH) {
		m_pBVH->ShowUI(this, ui);
	}

	ImGui::Checkbox("ShowSignedDistanceField", &m_ui.visibleSignedDistanceField);
	if (m_ui.visibleSignedDistanceField) {
		if(m_pSignedDistanceField == nullptr) m_pSignedDistanceField = new SignedDistanceField(this);
		m_pSignedDistanceField->ShowUI(this, ui);
	}

	if (ImGui::Checkbox("ShowMorton", &m_ui.visibleMorton)) {
		BuildMorton();
	}

	ImGui::Checkbox("ShowVoxel", &m_ui.voxel.visible);
	if (m_ui.voxel.visible) {
		if (m_pVoxelizer == nullptr) {
			m_pVoxelizer = new Voxelizer(this);
		}
		if (ImGui::SliderInt("Resolution (2^x)", &m_ui.voxel.resolute, 1, 9)) {
			m_pVoxelizer->Execute(1 << m_ui.voxel.resolute);
		}
		m_pVoxelizer->ShowUI(this, ui);
	}

	if (ImGui::Checkbox("CreateMST", &m_ui.mst.visible)) {
		if (!m_ui.mst.visible)
		{
			RemoveNode("CreateMST");
			RemoveNode("CreateMSTSeam");
		}
	}

	if (m_ui.mst.visible) {
		if (ImGui::SliderFloat("MSTWeight", &m_ui.mst.weight, 0.0f, 0.1f)) {
			Vector<Vector3> seam;
			auto mst = MeshAlgorithm::CreateKruskulMST(*GetData(), seam, m_ui.mst.weight);
			{
				auto pPoints = std::make_shared<Primitive>();
				pPoints->SetPosition(std::move(mst));
				pPoints->SetType(GL_LINES);
				auto pNode = std::make_shared<PrimitiveNode>("CreateMST", pPoints, ColorUtility::CreatePrimary(1));
				pNode->SetMatrix(GetMatrix());
				//AddNode(pNode);

			}
			{
				auto pPoints = std::make_shared<Primitive>();
				pPoints->SetPosition(std::move(seam));
				pPoints->SetType(GL_LINES);
				auto pNode = std::make_shared<PrimitiveNode>("CreateMSTSeam", pPoints, ColorUtility::CreatePrimary(2));
				pNode->SetMatrix(GetMatrix());
				AddNode(pNode);
			}
		}
	}

	if (ImGui::Checkbox("CreatePoissonSampleVolume", &m_ui.poisson.volume)) {
		if (m_ui.poisson.volume) {
			auto pPoints = std::make_shared<Primitive>();
			pPoints->SetPosition(MeshAlgorithm::CreatePoissonSampleVolume(*this));
			pPoints->SetType(GL_POINTS);
			auto pNode = std::make_shared<PrimitiveNode>("PoissonVolume", pPoints);
			pNode->SetMatrix(GetMatrix());
			AddNode(pNode);
		} else {
			RemoveNode("PoissonVolume");
		}
	}

	if (ImGui::Checkbox("CreatePoissonSampleOnFace", &m_ui.poisson.surface.create)) {
		if (m_ui.poisson.surface.create) {
			auto pPoints = std::make_shared<Primitive>();
			pPoints->SetPosition(MeshAlgorithm::CreatePoissonSampleOnFace(*m_pHalfEdge));
			pPoints->SetType(GL_POINTS);
			auto pNode = std::make_shared<PrimitiveNode>("PoissonPoint", pPoints);
			pNode->SetMatrix(GetMatrix());
			AddNode(pNode);
		} else {
			RemoveNode("PoissonPoint");
		}
	} 

	if (ImGui::Checkbox("EditVertex", &m_ui.editVertex)) {
		m_ui.visibleVertex = true;
		ui.SetCurrentController(m_pController);
	}
	
}
}
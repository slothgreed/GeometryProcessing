#include "HalfEdgeNode.h"
#include "SimpleShader.h"
#include "Utility.h"
#include "MeshletGenerator.h"
#include "ShapeDiameterFunction.h"
#include "Voxelizer.h"
namespace KI
{

HalfEdgeNode::HalfEdgeNode(const String& name, const Shared<HalfEdgeStruct>& pStruct)
	: RenderNode(name)
	, m_pHalfEdge(pStruct)
{
	m_pShapeDiameterFunction = new ShapeDiameterFunction(this);
	m_pVoxelizer = new Voxelizer(this);
	BuildGLBuffer();
	SetBoundBox(pStruct->CreateBDB());

	m_pickIds.face.begin = 0;
	m_pickIds.face.num = m_pHalfEdge->GetFaceNum();

	m_pickIds.edge.begin = m_pickIds.face.num;
	m_pickIds.edge.num = m_pHalfEdge->GetEdgeNum();

	m_pickIds.vertex.begin = m_pickIds.face.num + m_pickIds.edge.num;
	m_pickIds.vertex.num = m_pHalfEdge->GetPositionNum();
}

HalfEdgeNode::~HalfEdgeNode()
{

}

const HalfEdgeNode::HalfEdgeParts* HalfEdgeNode::HalfEdgeParts::Cast(const RenderParts* pParts)
{
	assert(dynamic_cast<const HalfEdgeNode::HalfEdgeParts*>(pParts));
	return static_cast<const HalfEdgeNode::HalfEdgeParts*>(pParts);
}

String HalfEdgeNode::HalfEdgeParts::ToString()
{
	std::stringstream ss;
	if (type == Face) {
		ss << "Face : " << parts;
	} else if (type == Edge) {
		ss << "Edge : " << parts;
	} else if (type == Vertex) {
		ss << "Vertex : " << parts;
	}
	return ss.str();
}
void HalfEdgeNode::BuildGLBuffer()
{
	m_gpu.position = std::make_unique<GLBuffer>();
	m_gpu.position->Create(m_pHalfEdge->GetPosition());
	m_gpu.normal = std::make_unique<GLBuffer>();
	m_gpu.normal->Create(m_pHalfEdge->GetNormal());
	m_gpu.faceIndexBuffer = std::make_unique<GLBuffer>();
	m_gpu.faceIndexBuffer->Create(m_pHalfEdge->CreateIndexBufferData());

}

void HalfEdgeNode::ShowNormal()
{
	if (!m_pVectorShader) { m_pVectorShader = std::make_unique<VertexVectorShader>(); m_pVectorShader->Build(); }
	m_pVectorShader->Use();
	m_pVectorShader->SetColor(Vector4(1, 0, 0, 1));
	m_pVectorShader->SetLength(m_ui.normalLength);
	m_pVectorShader->SetModel(GetMatrix());
	m_pVectorShader->SetPosition(m_gpu.position.get());
	m_pVectorShader->SetVector(m_gpu.normal.get());
	m_pVectorShader->DrawArray(GL_POINTS, m_gpu.position->Num());
}
void HalfEdgeNode::BuildEdge()
{
	if (m_gpu.edgeIndexBuffer) { return; }
	m_gpu.edgeIndexBuffer = std::make_unique<GLBuffer>();
	m_gpu.edgeIndexBuffer->Create(m_pHalfEdge->CreateEdgeIndexBufferData());
}

void HalfEdgeNode::BuildSDF()
{
	if (m_gpu.sdf) { return; }
	m_gpu.sdf = std::make_unique<GLBuffer>();
	m_gpu.sdf->Create(m_pShapeDiameterFunction->GetResultVertexColor());
}
void HalfEdgeNode::DrawNode(const DrawContext& context)
{
	if (!m_ui.visible) { return; }
	auto pResource = context.pResource;
	auto pSimpleShader = pResource->GetShaderTable()->GetSimpleShader();
	pSimpleShader->Use();
	pSimpleShader->SetPosition(m_gpu.position.get());
	pSimpleShader->SetCamera(pResource->GetCameraBuffer());
	pSimpleShader->SetModel(GetMatrix());
	pSimpleShader->SetColor(Vector3(0.7f, 0.7f, 1.0f));
	if (m_ui.visibleMesh) {
		pSimpleShader->DrawElement(GL_TRIANGLES, m_gpu.faceIndexBuffer.get());
	}

	if (m_ui.visibleEdge) {
		pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
		pSimpleShader->DrawElement(GL_LINES, m_gpu.edgeIndexBuffer.get());
	}

	if (m_ui.visibleVertex) {
		pSimpleShader->SetColor(Vector3(1.0f, 0.0f, 0.0f));
		pSimpleShader->DrawArray(GL_POINTS, m_pHalfEdge->GetPositionNum());
	}

	if (m_ui.visibleNormal) {
		ShowNormal();
	}

	if (m_ui.visibleSDF) {
		auto pVertexColor = pResource->GetShaderTable()->GetVertexColorShader();
		pVertexColor->Use();
		pVertexColor->SetPosition(m_gpu.position.get());
		pVertexColor->SetCamera(pResource->GetCameraBuffer());
		pVertexColor->SetModel(GetMatrix());
		pVertexColor->SetColor(m_gpu.sdf.get());
		pVertexColor->DrawArray(GL_POINTS, m_pHalfEdge->GetPositionNum());
	}


	auto pPrimitiveColorShader = pResource->GetShaderTable()->GetPrimitiveColorShader();
	//if (pPrimitiveColorShader &&  m_meshletGpu.shader) {
	//	pPrimitiveColorShader->Use();
	//	pPrimitiveColorShader->SetPosition(m_meshletGpu.position.get());
	//	pPrimitiveColorShader->SetColor(m_meshletGpu.color.get());
	//	pPrimitiveColorShader->SetViewProj(proj * view);
	//	pPrimitiveColorShader->SetModel(Matrix4x4(1.0f));
	//	pPrimitiveColorShader->DrawElement(GL_TRIANGLES, m_gpu.faceIndexBuffer.get());
	//}

	if (m_ui.visibleVoxel) {
		m_pVoxelizer->Draw(pResource->GetCameraBuffer()->Handle());
	}


	if (m_ui.visibleMeshlet && m_meshletGpu.shader) {
		m_meshletGpu.shader->Use();
		m_meshletGpu.shader->SetPosition(m_meshletGpu.position.get());
		m_meshletGpu.shader->SetMeshlet(m_meshletGpu.culster.get());
		m_meshletGpu.shader->SetIndex(m_meshletGpu.index.get());
		m_meshletGpu.shader->SetCamera(pResource->GetCameraBuffer());
		m_meshletGpu.shader->SetModel(Matrix4x4(1.0f));
		m_meshletGpu.shader->Draw(0, m_meshletGpu.culster->Num());
	}
	
}

void HalfEdgeNode::PickNode(const PickContext& context)
{
	if (!m_ui.visible) { return; }

	auto pResource = context.pResource;
	auto pPickShader = pResource->GetShaderTable()->GetPointPickShader();
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
		pPickShader->DrawArray(GL_POINTS, m_pHalfEdge->GetPositionNum());
	}
}

bool HalfEdgeNode::CollectPickedNode(PickResult& result)
{
	if (m_pickIds.face.Inner(result.id)) {
		int faceId = result.id - m_pickIds.face.begin;
		result.pResult[this] = std::make_unique<HalfEdgeParts>(HalfEdgeParts::Type::Face, faceId);
		return true;
	} else if (m_pickIds.edge.Inner(result.id)) {
		int edgeId = result.id - m_pickIds.edge.begin;
		result.pResult[this] = std::make_unique<HalfEdgeParts>(HalfEdgeParts::Type::Edge, edgeId);
		return true;
	} else if (m_pickIds.vertex.Inner(result.id)) {
		int vertexId = result.id - m_pickIds.vertex.begin;
		result.pResult[this] = std::make_unique<HalfEdgeParts>(HalfEdgeParts::Type::Vertex, vertexId);
		return true;
	}
	return false;
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
	if (drawParts->type == HalfEdgeParts::Face) {
		pSimpleShader->DrawElement(GL_TRIANGLES, m_gpu.faceIndexBuffer.get(), 3, drawParts->parts * 3);
	} else if (drawParts->type == HalfEdgeParts::Edge) {
		pSimpleShader->DrawElement(GL_LINES, m_gpu.edgeIndexBuffer.get(), 2, drawParts->parts * 2);
	} else if (drawParts->type == HalfEdgeParts::Vertex) {
		pSimpleShader->DrawArray(GL_POINTS, drawParts->parts, 1);
	}
	context.pResource->GL()->EnableDepth();

}


void HalfEdgeNode::ShowUI()
{
	ImGui::Checkbox("Visible", &m_ui.visible);
	ImGui::Text("PositionNum%d, EdgeNum%d,TriangleNum %d",
		m_pHalfEdge->GetPositionNum(),
		m_pHalfEdge->GetEdgeNum(),
		m_pHalfEdge->GetFaceNum());

	ImGui::Checkbox("ShowMesh", &m_ui.visibleMesh);
	if (ImGui::Checkbox("ShowEdge", &m_ui.visibleEdge)) {
		if (m_ui.visibleEdge) {
			BuildEdge();
		}
	}

	ImGui::Checkbox("ShowVertex", &m_ui.visibleVertex);


	ImGui::Checkbox("ShowNormal", &m_ui.visibleNormal);
	if (m_ui.visibleNormal) {
		ImGui::SliderFloat("NormalLength", &m_ui.normalLength, 0.0f, 1.0f);
	}

	ImGui::Checkbox("VisibleMeshlet", &m_ui.visibleMeshlet);
	if (ImGui::SliderInt("MeshLetGenerate", &m_ui.meshlet, 0, 14)) {
		MeshletGenerator exec;
		auto meshlet = exec.Execute(*m_pHalfEdge.get(), 7);
		m_meshletGpu.culster = std::make_unique<GLBuffer>();
		m_meshletGpu.culster->Create<glm::ivec4>(meshlet.data);
		m_meshletGpu.color = std::make_unique<GLBuffer>();
		m_meshletGpu.color->Create(meshlet.color);

		m_meshletGpu.index = std::make_unique<GLBuffer>();
		m_meshletGpu.index->Create<int>(meshlet.index);
		m_meshletGpu.position = std::make_unique<GLBuffer>();
		m_meshletGpu.position->Create(TypeConverter::Convert4f(m_pHalfEdge->GetPosition()));
		m_meshletGpu.shader = std::make_unique<MeshletShader>();
		m_meshletGpu.shader->Build();
	}

	if (ImGui::Checkbox("ShowSDF", &m_ui.visibleSDF)) {
		m_pShapeDiameterFunction->Execute();
		BuildSDF();
	}
	m_pShapeDiameterFunction->ShowUI();

	if (ImGui::Checkbox("ShowVoxel", &m_ui.visibleVoxel)) {
		m_pVoxelizer->Execute(m_gpu.position->Handle(), m_gpu.faceIndexBuffer->Handle());
	}

}
}
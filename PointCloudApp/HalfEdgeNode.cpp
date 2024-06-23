#include "HalfEdgeNode.h"
#include "SimpleShader.h"
#include "Utility.h"
#include "MeshletGenerator.h"
namespace KI
{

HalfEdgeNode::HalfEdgeNode(const String& name, const Shared<HalfEdgeStruct>& pStruct)
	: RenderNode(name)
	, m_pHalfEdge(pStruct)
	, m_pPosition(nullptr) 
{
	BuildGLBuffer();
}

HalfEdgeNode::~HalfEdgeNode()
{

}

void HalfEdgeNode::BuildGLBuffer()
{
	m_pPosition = std::make_unique<GLBuffer>();
	m_pPosition->Create(m_pHalfEdge->GetPosition());
	m_pFaceIndexBuffer = std::make_unique<GLBuffer>();
	m_pFaceIndexBuffer->Create(m_pHalfEdge->CreateIndexBufferData());

}

void HalfEdgeNode::ShowEdge()
{
	if (m_pEdgeIndexBuffer) { return; }
	m_pEdgeIndexBuffer = std::make_unique<GLBuffer>();
	m_pEdgeIndexBuffer->Create(m_pHalfEdge->CreateEdgeIndexBufferData());
}
void HalfEdgeNode::DrawNode(const DrawContext& context)
{
	auto pSimpleShader = context.pShaderTable->GetSimpleShader();
	pSimpleShader->Use();
	pSimpleShader->SetPosition(m_pPosition.get());
	pSimpleShader->SetViewProj(context.pCamera->Projection() * context.pCamera->ViewMatrix());
	pSimpleShader->SetModel(Matrix4x4(1.0f));
	pSimpleShader->SetColor(Vector3(0.7f, 0.7f, 1.0f));
	if (m_ui.visibleMesh) {
		pSimpleShader->DrawElement(GL_TRIANGLES, m_pFaceIndexBuffer.get());
	}

	if (m_ui.visibleEdge) {
		pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
		pSimpleShader->DrawElement(GL_LINES, m_pEdgeIndexBuffer.get());
	}

	auto pPrimitiveColorShader = context.pShaderTable->GetPrimitiveColorShader();
	//if (pPrimitiveColorShader &&  m_meshletGpu.shader) {
	//	pPrimitiveColorShader->Use();
	//	pPrimitiveColorShader->SetPosition(m_meshletGpu.position.get());
	//	pPrimitiveColorShader->SetColor(m_meshletGpu.color.get());
	//	pPrimitiveColorShader->SetViewProj(proj * view);
	//	pPrimitiveColorShader->SetModel(Matrix4x4(1.0f));
	//	pPrimitiveColorShader->DrawElement(GL_TRIANGLES, m_pFaceIndexBuffer.get());
	//}



	if (m_meshletGpu.shader) {
		m_meshletGpu.shader->Use();
		m_meshletGpu.shader->SetPosition(m_meshletGpu.position.get());
		m_meshletGpu.shader->SetMeshlet(m_meshletGpu.culster.get());
		m_meshletGpu.shader->SetIndex(m_meshletGpu.index.get());
		m_meshletGpu.shader->SetViewProj(context.pCamera->Projection() * context.pCamera->ViewMatrix());
		m_meshletGpu.shader->SetModel(Matrix4x4(1.0f));
		m_meshletGpu.shader->Draw(0, m_meshletGpu.culster->Num());
	}
	
}
void HalfEdgeNode::ShowUI()
{
	ImGui::Text("PositionNum%d, EdgeNum%d,TriangleNum %d",
		m_pHalfEdge->GetPositionNum(),
		m_pHalfEdge->GetEdgeNum(),
		m_pHalfEdge->GetFaceNum());

	ImGui::Checkbox("ShowMesh", &m_ui.visibleMesh);
	if (ImGui::Checkbox("ShowEdge", &m_ui.visibleEdge)) {
		if (m_ui.visibleEdge) {
			ShowEdge();
		}
	}

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
}
}
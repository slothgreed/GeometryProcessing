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
	m_pSimpleShader = GetResource()->GetShaderTable()->GetSimpleShader();
	m_pPrimitiveColorShader = GetResource()->GetShaderTable()->GetPrimitiveColorShader();
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
	m_pSimpleShader->Use();
	m_pSimpleShader->SetPosition(m_pPosition.get());
	m_pSimpleShader->SetViewProj(context.m_pCamera->Projection() * context.m_pCamera->ViewMatrix());
	m_pSimpleShader->SetModel(Matrix4x4(1.0f));
	m_pSimpleShader->SetColor(Vector3(0.7f, 0.7f, 1.0f));
	if (m_ui.visibleMesh) {
		m_pSimpleShader->DrawElement(GL_TRIANGLES, m_pFaceIndexBuffer.get());
	}

	if (m_ui.visibleEdge) {
		m_pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
		m_pSimpleShader->DrawElement(GL_LINES, m_pEdgeIndexBuffer.get());
	}

	//if (m_pPrimitiveColorShader &&  m_meshletGpu.shader) {
	//	m_pPrimitiveColorShader->Use();
	//	m_pPrimitiveColorShader->SetPosition(m_meshletGpu.position.get());
	//	m_pPrimitiveColorShader->SetColor(m_meshletGpu.color.get());
	//	m_pPrimitiveColorShader->SetViewProj(proj * view);
	//	m_pPrimitiveColorShader->SetModel(Matrix4x4(1.0f));
	//	m_pPrimitiveColorShader->DrawElement(GL_TRIANGLES, m_pFaceIndexBuffer.get());
	//}



	if (m_meshletGpu.shader) {
		m_meshletGpu.shader->Use();
		m_meshletGpu.shader->SetPosition(m_meshletGpu.position.get());
		m_meshletGpu.shader->SetMeshlet(m_meshletGpu.culster.get());
		m_meshletGpu.shader->SetIndex(m_meshletGpu.index.get());
		m_meshletGpu.shader->SetViewProj(context.m_pCamera->Projection() * context.m_pCamera->ViewMatrix());
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
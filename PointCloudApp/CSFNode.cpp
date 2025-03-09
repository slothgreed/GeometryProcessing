#include "CSFNode.h"
#include "SimpleShader.h"
namespace KI
{
class CSFShader : public IShadingShader
{
public:
	CSFShader() {}
	virtual ~CSFShader() {}

	enum UNIFORM
	{
		NODE,
		NUM
	};

	virtual ShaderPath GetShaderPath()
	{
		ShaderPath path;
		path.version = "version.h";
		path.header.push_back("common.h");
		path.header.push_back("csf/csf.h");
		path.shader[SHADER_PROGRAM_VERTEX] = "csf/csf.vert";
		path.shader[SHADER_PROGRAM_FRAG] = "csf/csf.frag";
		
		return path;
	}

	void FetchUniformLocation()
	{
		m_uniform[UNIFORM::NODE] = glGetUniformLocation(Handle(), "u_node");
	}

	void SetCameraBuffer(const GLBuffer* pBuffer)
	{
		BindShaderStorage(0, pBuffer->Handle());
	}

	void SetLightBuffer(const GLBuffer* pBuffer)
	{
		BindShaderStorage(1, pBuffer->Handle());
	}

	void SetMaterialBuffer(const GLBuffer* pBuffer)
	{
		BindShaderStorage(2, pBuffer->Handle());
	}
	void SetNodeBuffer(const GLBuffer* pBuffer)
	{
		BindShaderStorage(3, pBuffer->Handle());
	}


	void SetNode(const Vector2i& node)
	{
		BindUniform(m_uniform[UNIFORM::NODE], node);
	}

	void SetVertexBuffer(const GLBuffer* pBuffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, pBuffer->Handle());
		glBindVertexBuffer(0, pBuffer->Handle(), 0, pBuffer->SizeOfData());
		glEnableVertexAttribArray(0);
		glVertexAttribFormat(0, 4, GL_FLOAT, false, offsetof(CSFVertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(CSFVertex, normal));
		glVertexAttribBinding(0, 0);
		glVertexAttribBinding(1, 0);
		glDisableVertexAttribArray(2); // 仮コード/別箇所でAttributeがEnableになる。
		OUTPUT_GLERROR;
	}

	void Draw(const CSFDrawRange& drawRange)
	{
		if (drawRange.draw.count == 0) return;
		DrawElementsBaseVertex(drawRange.draw);
		OUTPUT_GLERROR;
	}
private:
	GLuint m_uniform[UNIFORM::NUM];
};

const CSFRenderNode::CSFRenderParts* CSFRenderNode::CSFRenderParts::Cast(const RenderParts* pParts)
{
	assert(dynamic_cast<const CSFRenderNode::CSFRenderParts*>(pParts));
	return static_cast<const CSFRenderNode::CSFRenderParts*>(pParts);

}
String CSFRenderNode::CSFRenderParts::ToString()
{
	std::stringstream ss;
	
	if (type == Face) {
		ss << "Face :";
	} else if (type == Edge) {
		ss << "Edge :";
	}

	ss << parts << "\n";
	ss << "Geometry Index" << m_pNode->m_solids[parts].geomIndex << "\n";
	ss << "Matrix Index" << m_pNode->m_solids[parts].matrixIndex << "\n";
	ss << "Material Index" << m_pNode->m_solids[parts].materialIndex << "\n";
	return ss.str();

}
CSFRenderNode::CSFRenderNode(const String& name)
	: RenderNode(name)
	, m_pShader(nullptr)
	, m_needUpdateProperty(true)
{
};
CSFRenderNode::~CSFRenderNode()
{
	RELEASE_INSTANCE(m_pShader);
}

void CSFRenderNode::UpdateProperty()
{
	if (!m_needUpdateProperty) { return; }
	m_property = Property();

	for (const auto& solid : m_solids) {
		m_property.triangleNum += m_solidDraw[solid.geomIndex][solid.drawIndex].draw.count;
		m_property.lineNum += m_wireDraw[solid.geomIndex][solid.drawIndex].draw.count;
	}

	m_property.triangleNum /= 3;
	m_property.lineNum /= 2;
	m_needUpdateProperty = true;
}
void CSFRenderNode::PickNode(const PickContext& context)
{
	if (!m_ui.visible) { return; }
	auto pResource = context.pResource;
	auto pPickShader = pResource->GetShaderTable()->GetPointPickByID();

	VertexFormat format;
	format.type = DATA_FLOAT;
	format.offset = offsetof(CSFVertex, position);
	format.normalized = false;
	format.componentSize = 4;

	pPickShader->Use();
	pPickShader->SetCamera(pResource->GetCameraBuffer());
	pPickShader->SetPosition(format, m_gpu.pMeshBuffer[0]->pVertex.get());
	pPickShader->BindIndexBuffer(m_gpu.pMeshBuffer[0]->pIndex.get());
	int i = 0;
	for (const auto& solid : m_solids) {
		pPickShader->SetModel(m_nodes[solid.matrixIndex].world);
		pPickShader->SetPickOffset(i++);
		pPickShader->DrawElementsBaseVertex(m_solidDraw[solid.geomIndex][solid.drawIndex].draw);
	}

	for (const auto& solid : m_solids) {
		pPickShader->SetModel(m_nodes[solid.matrixIndex].world);
		pPickShader->SetPickOffset(i++);
		pPickShader->DrawElementsBaseVertex(m_wireDraw[solid.geomIndex][solid.drawIndex].draw);
	}
}


bool CSFRenderNode::CollectPickedNode(PickResult& result)
{
	int resultId = result.id;
	if (resultId < m_solids.size()) {
		result.pResult[this] = std::make_unique<CSFRenderParts>(this, CSFRenderParts::Face, resultId);
		return true;
	}

	resultId = result.id - m_solids.size();
	if (resultId < m_solids.size()) {
		result.pResult[this] = std::make_unique<CSFRenderParts>(this, CSFRenderParts::Edge, resultId);
		return true;
	}

	return false;
}
void CSFRenderNode::DrawPartsNode(const DrawContext& context, const RenderParts& parts)
{
	auto pResource = context.pResource;
	auto drawParts = CSFRenderParts::Cast(&parts);
	m_pShader->Use();
	m_pShader->SetCameraBuffer(context.pResource->GetCameraBuffer());
	m_pShader->SetLightBuffer(context.pResource->GetLightBuffer());
	m_pShader->SetMaterialBuffer(m_gpu.pMaterialBuffer.get());
	m_pShader->SetNodeBuffer(m_gpu.pNodeBuffer.get());
	m_pShader->SetVertexBuffer(m_gpu.pMeshBuffer[0]->pVertex.get());
	m_pShader->BindIndexBuffer(m_gpu.pMeshBuffer[0]->pIndex.get());
	const auto& solid = m_solids[drawParts->parts];
	m_pShader->SetNode(Vector2i(solid.matrixIndex, solid.materialIndex));
	if (drawParts->type == CSFRenderParts::Face) {
		m_pShader->Draw(m_solidDraw[solid.geomIndex][solid.drawIndex]);
	} else {
		m_pShader->Draw(m_wireDraw[solid.geomIndex][solid.drawIndex]);
	}
}

void CSFRenderNode::UpdateMaterial()
{
	if (m_gpu.pMaterialBuffer == nullptr) {
		m_gpu.pMaterialBuffer = std::make_unique<GLBuffer>();
		m_gpu.pMaterialBuffer->Create<KI::CSFMaterial>(m_materials);
	}
}

void CSFRenderNode::UpdateNode()
{
	if (m_gpu.pNodeBuffer == nullptr) {
		m_gpu.pNodeBuffer = std::make_unique<GLBuffer>();
		m_gpu.pNodeBuffer->Create<KI::CSFNode>(m_nodes);
	}
}

void CSFRenderNode::DrawNode(const DrawContext& context)
{
	if (!m_ui.visible)return;
	UpdateProperty();
	if (!m_pShader) {
		m_pShader = new CSFShader();
		m_pShader->Build();
	}

	context.pResource->GL()->DisableCullFace();
	m_pShader->Use();
	m_pShader->SetCameraBuffer(context.pResource->GetCameraBuffer());
	m_pShader->SetLightBuffer(context.pResource->GetLightBuffer());
	m_pShader->SetMaterialBuffer(m_gpu.pMaterialBuffer.get());
	m_pShader->SetNodeBuffer(m_gpu.pNodeBuffer.get());
	m_pShader->SetVertexBuffer(m_gpu.pMeshBuffer[0]->pVertex.get());
	m_pShader->BindIndexBuffer(m_gpu.pMeshBuffer[0]->pIndex.get());

	for (const auto& solid : m_solids) {
		if (m_ui.visibleGeomIndex != -1 && solid.geomIndex != m_ui.visibleGeomIndex) continue;
		Vector2i value = Vector2i(solid.matrixIndex, solid.materialIndex);
		m_pShader->SetNode(value);
		m_pShader->Draw(m_solidDraw[solid.geomIndex][solid.drawIndex]);
		if (m_ui.showWire) {
			m_pShader->SetNode(Vector2i(solid.matrixIndex, 0));
			m_pShader->Draw(m_wireDraw[solid.geomIndex][solid.drawIndex]);
		}
	}
}

void CSFRenderNode::ShowUI()
{
	ImGui::Checkbox("Visible", &m_ui.visible);
	ImGui::Checkbox("Visible Wire", &m_ui.showWire);
	ImGui::SliderInt("VisibleIndex", &m_ui.visibleGeomIndex, -1, m_solidDraw.size());
	ImGui::Text("Parts Num %d\n Geometry Num %d\n", m_solids.size(), m_solidDraw.size());
	ImGui::Text("Triangle Num %d\n Line Num %d\n", m_property.triangleNum, m_property.lineNum);

}
}
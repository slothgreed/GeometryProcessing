#include "PrimitiveNode.h"
#include "PointCloud.h"
#include "VertexColorShader.h"
#include "SimpleShader.h"

PrimitiveNode::PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive, const glm::vec3& color)
	: RenderNode(name)
	, m_color(color)
{
	m_pShader = dynamic_pointer_cast<SimpleShader>(GetResource()->GetShader(IShadingShader::Type::Simple));
	m_pPrimitive = std::move(pPrimitive);
	BuildGLBuffer();
}

PrimitiveNode::PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive)
	: RenderNode(name)
{
	assert(pPrimitive->Color().size());
	m_pShader = dynamic_pointer_cast<VertexColorShader>(GetResource()->GetShader(IShadingShader::Type::VertexColor));
	m_pPrimitive = std::move(pPrimitive);
	BuildGLBuffer();
}


PrimitiveNode::~PrimitiveNode()
{

}

void PrimitiveNode::BuildGLBuffer()
{
	if (!m_pPrimitive->NeedUpdate()) { return; }

	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(m_pPrimitive->Position());

	if (m_pPrimitive->Color().size() != 0) {
		m_pColorBuffer = std::make_unique<GLBuffer>();
		m_pColorBuffer->Create(m_pPrimitive->Color());
	}

	if (m_pPrimitive->Index().size() != 0) {
		m_pIndexBuffer = std::make_unique<GLBuffer>();
		m_pIndexBuffer->Create(m_pPrimitive->Index());
	}
	m_pPrimitive->ClearUpdate();


}
const Shared<Primitive>& PrimitiveNode::GetData() const
{
	return m_pPrimitive;
}
void PrimitiveNode::UpdateData()
{
	m_pPrimitive->Update();
}
void PrimitiveNode::UpdateRenderData()
{
	BuildGLBuffer();
}


void PrimitiveNode::DrawData(const mat4x4& proj, const mat4x4& view)
{
	UpdateRenderData();
	m_pShader->Use();
	if (m_pShader->GetType() == IShadingShader::Type::Simple) {
		auto pShader = (SimpleShader*)m_pShader.get();
		pShader->SetupVertexAttribArray(m_pPositionBuffer.get());
		pShader->SetViewProj(proj * view);
		pShader->SetModel(glm::mat4x4(1.0f));
		pShader->SetColor(m_color);
	} else 	if (m_pShader->GetType() == IShadingShader::Type::VertexColor) {
		auto pShader = (VertexColorShader*)m_pShader.get();
		pShader->SetPosition(m_pPositionBuffer.get());
		pShader->SetColor(m_pColorBuffer.get());

		pShader->SetViewProj(proj * view);
		pShader->SetModel(glm::mat4x4(1.0f));
	}

	if (m_pIndexBuffer) {
		m_pShader->DrawElement(m_pPrimitive->GetType(), m_pIndexBuffer.get());
	} else {
		m_pShader->DrawArray(m_pPrimitive->GetType(), m_pPositionBuffer.get());
	}
}
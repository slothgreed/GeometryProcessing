#include "PrimitiveNode.h"
#include "PointCloud.h"
#include "VertexColorShader.h"
#include "SimpleShader.h"

PrimitiveNode::PrimitiveNode(const string& name, shared_ptr<Primitive>& pPrimitive, const glm::vec3& color)
	: RenderNode(name)
	, m_color(color)
{
	m_pShader = dynamic_pointer_cast<SimpleShader>(GetResource()->GetShader(IShader::Type::Simple));
	m_pPrimitive = std::move(pPrimitive);
	BuildGLBuffer();
}

PrimitiveNode::PrimitiveNode(const string& name, shared_ptr<Primitive>& pPrimitive)
	: RenderNode(name)
{
	assert(pPrimitive->Color().size());
	m_pShader = dynamic_pointer_cast<VertexColorShader>(GetResource()->GetShader(IShader::Type::VertexColor));
	m_pPrimitive = std::move(pPrimitive);
	BuildGLBuffer();
}


PrimitiveNode::~PrimitiveNode()
{

}

void PrimitiveNode::BuildGLBuffer()
{
	if (!m_pPrimitive->NeedUpdate()) { return; }

	m_pPositionBuffer = std::make_unique<GLBuffer>(GL_ARRAY_BUFFER);
	m_pPositionBuffer->Create(m_pPrimitive->Position());

	if (m_pPrimitive->Color().size() != 0) {
		m_pColorBuffer = std::make_unique<GLBuffer>(GL_ARRAY_BUFFER);
		m_pColorBuffer->Create(m_pPrimitive->Color());
	}

	if (m_pPrimitive->Index().size() != 0) {
		m_pIndexBuffer = std::make_unique<GLBuffer>(GL_ELEMENT_ARRAY_BUFFER);
		m_pIndexBuffer->Create(m_pPrimitive->Index());
	}
	m_pPrimitive->ClearUpdate();


}
const std::shared_ptr<Primitive>& PrimitiveNode::GetData() const
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
	if (m_pShader->GetType() == IShader::Type::Simple) {
		auto pShader = (SimpleShader*)m_pShader.get();
		pShader->SetupVertexAttribute();
		pShader->SetViewProj(proj * view);
		pShader->SetModel(glm::mat4x4(1.0f));
		pShader->SetColor(m_color);
		//ATTRIB_POSITION
		glBindVertexBuffer(0, m_pPositionBuffer->GetId(), 0, sizeof(glm::vec3));
		OUTPUT_GLERROR;
	} else 	if (m_pShader->GetType() == IShader::Type::VertexColor) {
		auto pShader = (VertexColorShader*)m_pShader.get();
		pShader->SetupVertexAttribute();
		pShader->SetViewProj(proj * view);
		pShader->SetModel(glm::mat4x4(1.0f));
		//ATTRIB_POSITION
		glBindVertexBuffer(0, m_pPositionBuffer->GetId(), 0, sizeof(glm::vec3));
		OUTPUT_GLERROR;

		//ATTRIB_COLOR
		glBindVertexBuffer(1, m_pColorBuffer->GetId(), 0, sizeof(glm::vec3));
		OUTPUT_GLERROR;
	}

	if (m_pIndexBuffer) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pIndexBuffer->GetId());
		glDrawElements(m_pPrimitive->GetType(), m_pIndexBuffer->Size(), GL_UNSIGNED_INT, 0);
	} else {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDrawArrays(m_pPrimitive->GetType(), 0, m_pPositionBuffer->Size());
	}
	OUTPUT_GLERROR;
}
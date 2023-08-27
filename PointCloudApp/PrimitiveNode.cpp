#include "PrimitiveNode.h"
#include "PointCloud.h"
PrimitiveNode::PrimitiveNode(unique_ptr<Primitive>&& pPrimitive)
	:m_pShader(std::make_unique<SimpleShader>())
{
	m_pShader->Build();
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

	m_pColorBuffer = std::make_unique<GLBuffer>(GL_ARRAY_BUFFER);
	m_pColorBuffer->Create(m_pPrimitive->Color());
	m_pPrimitive->ClearUpdate();
}
const std::unique_ptr<Primitive>& PrimitiveNode::GetData() const
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
	m_pShader->SetupVertexAttribute();
	m_pShader->SetViewProj(proj * view);
	m_pShader->SetModel(glm::mat4x4(1.0f));

	//ATTRIB_POSITION
	glBindVertexBuffer(0, m_pPositionBuffer->GetId(), 0, sizeof(glm::vec3));
	OUTPUT_GLERROR;

	//ATTRIB_COLOR
	glBindVertexBuffer(1, m_pColorBuffer->GetId(), 0, sizeof(glm::vec3));
	OUTPUT_GLERROR;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDrawArrays(GL_POINTS, 0, m_pPositionBuffer->Size());
	OUTPUT_GLERROR;
}
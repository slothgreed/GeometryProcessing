#include "PointCloudNode.h"
#include "PointCloud.h"
PointCloudNode::PointCloudNode()
	:m_pShader(std::make_unique<SimpleShader>())
{
	m_pShader->Build();
}

PointCloudNode::~PointCloudNode()
{

}

void PointCloudNode::Set(unique_ptr<PointCloud>&& pPointCloud)
{
	m_pPointCloud = std::move(pPointCloud);
	BuildGLBuffer();
}

void PointCloudNode::BuildGLBuffer()
{
	if (!m_pPointCloud->NeedUpdate()) { return; }

	m_pPositionBuffer = std::make_unique<GLBuffer>(GL_ARRAY_BUFFER);
	m_pPositionBuffer->Create(m_pPointCloud->Position());

	m_pColorBuffer = std::make_unique<GLBuffer>(GL_ARRAY_BUFFER);
	m_pColorBuffer->Create(m_pPointCloud->Color());
	m_pPointCloud->ClearUpdate();
}
const std::unique_ptr<PointCloud>& PointCloudNode::GetData() const
{ 
	return m_pPointCloud;
}
void PointCloudNode::UpdateData()
{
	m_pPointCloud->Update();
}
void PointCloudNode::UpdateRenderData()
{
	BuildGLBuffer();
}


void PointCloudNode::Draw(const mat4x4& proj, const mat4x4& view)
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
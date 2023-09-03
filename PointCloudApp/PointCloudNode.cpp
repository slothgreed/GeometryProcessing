#include "PointCloudNode.h"
#include "PointCloud.h"
#include "KDTree.h"
#include "KDTreeNanoFlann.h"

PointCloudNode::PointCloudNode(const string& name, std::shared_ptr<PointCloud>& pPrimitive)
	:RenderNode(name)
{
	m_pPointCloud = std::move(pPrimitive);
	m_pShader = dynamic_pointer_cast<VertexColorShader>(GetResource()->GetShader(IShader::Type::VertexColor));
	m_algorithm[ALGORITHM_KDTREE] = new KDTreeNanoFlann(this, 3);
	//m_algorithm[ALGORITHM_KDTREE] = new KDTree(this, 3);

	BuildGLBuffer();
}

PointCloudNode::~PointCloudNode()
{
	for (auto& algorithm : m_algorithm) {
		delete algorithm.second;
		algorithm.second = nullptr;
	}
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
const std::shared_ptr<PointCloud>& PointCloudNode::GetData() const
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

void PointCloudNode::ShowUI()
{
	for (auto& algorithm : m_algorithm) {
		algorithm.second->ShowUI();
	}
}
void PointCloudNode::DrawData(const mat4x4& proj, const mat4x4& view)
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
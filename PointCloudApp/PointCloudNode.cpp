#include "PointCloudNode.h"
#include "PointCloud.h"
#include "KDTree.h"
#include "KDTreeNanoFlann.h"
#include "Utility.h"
#include "Harris3D.h"
#include "AlphaShape.h"
#include "DelaunayGenerator.h"
#include <Eigen/SVD>
#include <Eigen/Core>
namespace KI
{

PointCloudNode::PointCloudNode(const String& name, Shared<PointCloud>& pPrimitive)
	:RenderNode(name)
{
	m_pPointCloud = std::move(pPrimitive);
	m_algorithm[ALGORITHM_KDTREE] = new KDTreeNanoFlann(this, 3);
	m_algorithm[ALGORITHM_HARRIS3D] = new Harris3D(this);
	m_algorithm[ALGORITHM_ALPHASHAPE] = new AlphaShape2D(this);
	m_algorithm[ALGORITHM_DELAUNAY] = new DelaunayGenerator(this);
	//m_algorithm[ALGORITHM_KDTREE] = new KDTree(this, 3);
	m_normal = m_pPointCloud->Normal();
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

	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(m_pPointCloud->Position());

	if (m_pPointCloud->Color().size() != 0) {
		UpdateColor(m_pPointCloud->Color());
	}

	m_pPointCloud->ClearUpdate();
}
const Shared<PointCloud>& PointCloudNode::GetData() const
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
const Vector<Vector3>& PointCloudNode::GetNormal()
{
	ComputeNormal();
	return m_normal;
}

const Vector<int>& PointCloudNode::GetNeighbor(int index)
{
	ComputeNeighbor(10);

	return m_neighbor[index];
}

void PointCloudNode::UpdateColor(const Vector<Vector4>& color)
{
	if (!m_pColorBuffer) {
		m_pColorBuffer = std::make_unique<GLBuffer>();
	}

	m_pColorBuffer->Create(color);
}
void PointCloudNode::ShowUI()
{
	for (auto& algorithm : m_algorithm) {
		algorithm.second->ShowUI();
	}

	if (ImGui::Button("ShowNormal")) {
		ShowNormal();
	}

	if (ImGui::Button("ReverseNormal")) {
		auto normal = m_pPointCloud->Color();
		for (int i = 0; i < normal.size(); i++) {
			normal[i] = -normal[i];
		}

		UpdateColor(normal);
	}
}
void PointCloudNode::ComputeNormal()
{
	if (m_normal.size() != 0) { return; }
	ComputeNeighbor(10);
	m_normal.resize(m_pPointCloud->Position().size());
	for (size_t i = 0; i < m_neighbor.size(); i++) {
		const auto& pos = m_pPointCloud->Position()[i];
		Eigen::MatrixXf matrix(3, m_neighbor[i].size());
		for (size_t j = 0; j < m_neighbor[i].size(); j++) {
			Vector3 center = m_pPointCloud->Position()[m_neighbor[i][j]] - pos;
			matrix(0, j) = center.x;
			matrix(1, j) = center.y;
			matrix(2, j) = center.z;
		}

		Eigen::JacobiSVD<Eigen::MatrixXf> svd(matrix, Eigen::ComputeThinU);
		Eigen::Vector3f normal = svd.matrixU().col(2);
		m_normal[i][0] = normal(0);
		m_normal[i][1] = normal(1);
		m_normal[i][2] = normal(2);
		m_normal[i] = glm::normalize(m_normal[i]);
	}
}
void PointCloudNode::ShowNormal()
{
	ComputeNormal();
	auto normal = m_pPointCloud->Normal();
	Vector<Vector4> normal4;
	for (const auto& c : normal) {
		normal4.push_back(Vector4(c, 1.0));
	}

	UpdateColor(normal4);
}
void PointCloudNode::ComputeNeighbor(float radius)
{
	if (m_neighbor.size() != 0) { return; }
	KDTreeNanoFlann* pFlann = (KDTreeNanoFlann*)m_algorithm[ALGORITHM_KDTREE];
	pFlann->Execute();
	m_neighbor.resize(m_pPointCloud->Position().size());
	for (size_t i = 0; i < m_pPointCloud->Position().size(); i++) {
		m_neighbor[i] = pFlann->GetRadiusNeighbor(m_pPointCloud->Position()[i], radius);
	}
}

void PointCloudNode::ComputeTangent()
{
	ComputeNormal();
	if (m_tangentX.size() == 0) { return; }
	for (int i = 0; i < m_normal.size(); i++) {
		Vector3 test(1, 0, 0);
		if (std::fabs(glm::dot(test, m_normal[i])) > 0.9f) {
			test = Vector3(0, 1, 0);
		}

		m_tangentX.push_back(glm::normalize(glm::cross(test, m_normal[i])));
		m_tangentY.push_back(glm::normalize(glm::cross(m_tangentX[i], m_normal[i])));
	}
}
void PointCloudNode::DrawNode(const DrawContext& context)
{
	UpdateRenderData();
	auto pResource = context.pResource;
	if (m_pColorBuffer) {
		auto pShader = pResource->GetShaderTable()->GetVertexColorShader();
		pShader->Use();
		pShader->SetPosition(m_pPositionBuffer.get());
		pShader->SetColor(m_pColorBuffer.get());
		pShader->SetCamera(pResource->GetCameraBuffer());
		pShader->SetModel(Matrix4x4(1.0f));
		pShader->DrawArray(GL_POINTS, m_pPositionBuffer.get());
	} else {
		glPointSize(5.0f);
		auto pShader = pResource->GetShaderTable()->GetSimpleShader();
		pShader->Use();
		pShader->SetPosition(m_pPositionBuffer.get());
		pShader->SetColor(Vector3(1, 0, 0));
		pShader->SetCamera(pResource->GetCameraBuffer());
		pShader->SetModel(Matrix4x4(1.0f));
		pShader->DrawArray(GL_POINTS, m_pPositionBuffer.get());

	}
}
}
#include "Harris3D.h"
#include "PointCloud.h"
#include "PointCloudNode.h"
#include "PrimitiveNode.h"
#include "Utility.h"

namespace KI
{
Harris3D::Harris3D(PointCloudNode* pointCloud)
	:m_pPointCloud(pointCloud)
{
	m_min = std::numeric_limits<float>::max();
	m_max = std::numeric_limits<float>::min();
}

Harris3D::~Harris3D()
{

}

void Harris3D::Execute()
{
	int posNum = m_pPointCloud->GetData()->Position().size();
	m_result.resize(posNum);
	for (int i = 0; i < m_pPointCloud->GetData()->Position().size(); i++) {
		const auto& neighbor = m_pPointCloud->GetNeighbor(i);
		const auto& normal = m_pPointCloud->GetNormal();
		Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(3, 3);
		float mat[9] = { 0 };
		for (int j = 0; j < neighbor.size(); j++) {
			matrix(0, 0) += normal[j].x - normal[i].x;
			matrix(1, 0) += normal[j].x - normal[i].y;
			matrix(2, 0) += normal[j].x - normal[i].z;
			matrix(0, 1) += normal[j].y - normal[i].x;
			matrix(1, 1) += normal[j].y - normal[i].y;
			matrix(2, 1) += normal[j].y - normal[i].z;
			matrix(0, 2) += normal[j].z - normal[i].x;
			matrix(1, 2) += normal[j].z - normal[i].y;
			matrix(2, 2) += normal[j].z - normal[i].z;

			mat[0] += normal[j].x - normal[i].x;
			mat[1] += normal[j].x - normal[i].y;
			mat[2] += normal[j].x - normal[i].z;
			mat[3] += normal[j].y - normal[i].x;
			mat[4] += normal[j].y - normal[i].y;
			mat[5] += normal[j].y - normal[i].z;
			mat[6] += normal[j].z - normal[i].x;
			mat[7] += normal[j].z - normal[i].y;
			mat[8] += normal[j].z - normal[i].z;
		}

		matrix(0, 0) /= posNum; matrix(0, 1) /= posNum; matrix(0, 2) /= posNum;
		matrix(1, 0) /= posNum; matrix(1, 1) /= posNum; matrix(1, 2) /= posNum;
		matrix(2, 0) /= posNum; matrix(2, 1) /= posNum; matrix(2, 2) /= posNum;

		mat[0] /= posNum; mat[1] /= posNum; mat[2] /= posNum;
		mat[3] /= posNum; mat[4] /= posNum; mat[5] /= posNum;
		mat[6] /= posNum; mat[7] /= posNum; mat[8] /= posNum;

		float det1 = matrix.determinant();
		float tr1 = matrix.trace();

		float det0 = 
			mat[0] * mat[4] * mat[8] -
			mat[0] * mat[5] * mat[7] +
			mat[1] * mat[5] * mat[6] -
			mat[1] * mat[3] * mat[8] +
			mat[2] * mat[3] * mat[7] -
			mat[2] * mat[4] * mat[6];
		float tr0 = mat[0] + mat[4] + mat[8];
		//m_result[i] = matrix.determinant() / matrix.trace();
		m_result[i] = det0 - 0.04f * tr0 * tr0;
		m_result[i] = det1 - 0.04f * tr1 * tr1;
		m_min = std::min(m_result[i], m_min);
		m_max = std::max(m_result[i], m_max);
	}
}

void Harris3D::ShowUI(RenderNode* pNode, UIContext& ui)
{
	if (ImGui::Button("ShowCorner:Harris3D")) {
		if (m_result.size() == 0) { Execute(); }
		Vector<Vector3> color(m_result.size(), Vector3(1, 1, 1));
		for (int i = 0; i < m_result.size(); i++) {
			if (m_result[i] < -0.01) {
				color[i] = Vector3(1, 0, 0);
			}
		}

		m_pPointCloud->GetData()->SetColor(std::move(color));
		m_pPointCloud->GetData()->Update();
	}
}
}
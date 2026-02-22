#include "ShapeMatching.h"
#include "KMeansAlgorithm.h"
namespace KI
{

Eigen::Matrix3f ToEigen(const Matrix4x4& M)
{
	Eigen::Matrix3f A;
	A <<
		M[0][0], M[1][0], M[2][0],
		M[0][1], M[1][1], M[2][1],
		M[0][2], M[1][2], M[2][2];
	return A;
}

Matrix4x4 ToGlm(const Eigen::Matrix3f& R)
{
	Matrix4x4 out(1.0f);

	out[0][0] = R(0, 0); out[1][0] = R(0, 1); out[2][0] = R(0, 2);
	out[0][1] = R(1, 0); out[1][1] = R(1, 1); out[2][1] = R(1, 2);
	out[0][2] = R(2, 0); out[1][2] = R(2, 1); out[2][2] = R(2, 2);

	return out;
}
Matrix4x4 ComputeRotationFromSVD(const Matrix4x4& M)
{
	Eigen::JacobiSVD<Eigen::Matrix3f> svd(
		ToEigen(M), Eigen::ComputeFullU | Eigen::ComputeFullV);

	Eigen::Matrix3f U = svd.matrixU();
	Eigen::Matrix3f V = svd.matrixV();

	Eigen::Matrix3f R = U * V.transpose();

	if (R.determinant() < 0.0f) {
		Eigen::Matrix3f U2 = U;
		U2.col(2) *= -1.0f;
		R = U2 * V.transpose();
	}

	// 平行移動は呼び出し側で足す or そのまま
	// out[3] = M[3];

	return ToGlm(R);
}

Vector3 ShapeMatching::ComputeCenter(const Vector<VertexWeight>& cluster, bool init)
{
	Vector3 center = Vector3(0.0);
	float sumW = 0.0f;

	for (size_t i = 0; i < cluster.size(); i++) {
		if (init) {
			center += cluster[i].weight * m_particle.m_initPoint[cluster[i].index];
		} else {
			center += cluster[i].weight * m_particle.m_position[cluster[i].index];
		}
		sumW += cluster[i].weight;
	}

	if (sumW == 0.0) { return center; }
	return center / sumW;
}

Matrix4x4 ShapeMatching::ComputeApq(const Vector<VertexWeight>& cluster, const Vector3& cp,	const Vector3& cq)
{
	Matrix4x4 A(0.0f);

	for (int i = 0; i < cluster.size(); i++) {
		auto pi = m_particle.m_position[cluster[i].index] - cp;
		auto qi = m_particle.m_initPoint[cluster[i].index] - cq;

		A[0] += Vector4(cluster[i].weight * pi.x * qi, 0.0f);
		A[1] += Vector4(cluster[i].weight * pi.y * qi, 0.0f);
		A[2] += Vector4(cluster[i].weight * pi.z * qi, 0.0f);
	}
	return A;
}
ShapeMatching::ShapeMatching(HalfEdgeNode* pNode)
	:m_pHalfEdge(pNode)
	, m_clusterNum(10)
{
}
ShapeMatching::~ShapeMatching()
{
}


void ShapeMatching::Initialize()
{
	const int MIN_CLUSTER_NUM = 2; // 1つの頂点は必ず minCluster 個のクラスタには所属させる.
	auto bdb = m_pHalfEdge->GetData()->CreateBDB();
	float sigma = bdb.MaxLength();
	KMeansAlgorithm kmeans;
	kmeans.Execute(m_pHalfEdge->GetData()->GetVertex(), m_clusterNum, 10);
	m_cluster.resize(kmeans.GetSeeds().size());
	m_particle.Allocate(m_pHalfEdge->GetData()->GetVertex().size());
	for (size_t i = 0; i < m_pHalfEdge->GetData()->GetVertex().size(); i++) {
		const auto& vertexs = m_pHalfEdge->GetData()->GetVertex();
		std::vector<std::pair<float, int>> distances;
		for (size_t j = 0; j < kmeans.GetSeeds().size(); j++) {
			float dist = glm::length2(vertexs[i] - kmeans.GetSeeds()[j]);
			distances.emplace_back(dist, j);
		}
		std::nth_element(distances.begin(),
			distances.begin() + MIN_CLUSTER_NUM,
			distances.end());

		float sumWeight = 0.0f;
		for (int j = 0; j < MIN_CLUSTER_NUM; j++) {
			sumWeight += std::exp(-distances[j].first / (2 * sigma * sigma));
		}

		for (int j = 0; j < MIN_CLUSTER_NUM; j++) {
			float weight = std::exp(-distances[j].first / (2 * sigma * sigma));
			m_cluster[distances[j].second].push_back(VertexWeight(i, weight));
		}

		m_particle.m_beforePosition[i] = vertexs[i];
		m_particle.m_position[i] = vertexs[i];
		m_particle.m_initPoint[i] = vertexs[i];
	}

	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(DATA_TYPE::DATA_FLOAT, m_particle.Size(), sizeof(Vector3), NULL);
	m_pNormalBuffer = std::make_unique<GLBuffer>();
	m_pNormalBuffer->Create(DATA_TYPE::DATA_FLOAT, m_particle.Size(), sizeof(Vector3), NULL);
}
void ShapeMatching::Update(float dt)
{
	UpdateCpu(dt);
	UpdateGpu();
}
void ShapeMatching::UpdateCpu(float dt)
{
	
	const Vector3 gravity = Vector3(0, -9.8, 0);
	const Vector3 floor = Vector3(0, -50.0, 0);
	const float alpha = 0.5f;
	for (size_t i = 0; i < m_particle.Size(); i++) {
		m_particle.m_velocity[i] += dt * gravity;
		m_particle.m_beforePosition[i] = m_particle.m_position[i];
		m_particle.m_position[i] += dt * m_particle.m_velocity[i];
	}

	for (const auto& c : m_cluster) {
		auto cp = ComputeCenter(c, false);
		auto cq = ComputeCenter(c, true);

		auto Apq = ComputeApq(c, cp, cq);
		auto R = ComputeRotationFromSVD(Apq);

		for (size_t i = 0; i < c.size(); i++) {
			auto g = cp + Vector3(R * Vector4(m_particle.m_initPoint[c[i].index] - cq, 1.0f));
			m_particle.m_position[c[i].index] = glm::mix(m_particle.m_position[c[i].index], g, alpha);
		}
	}

	for (size_t i = 0; i < m_particle.Size(); i++) {
		m_particle.m_velocity[i] = (m_particle.m_position[i] - m_particle.m_beforePosition[i]) / dt;
		if (m_particle.m_position[i].y < floor.y) {
			m_particle.m_position[i].y = floor.y;
			m_particle.m_velocity[i].y = 0.0f;
		}
	}

	UpdateGpu();
}

void ShapeMatching::UpdateGpu()
{
	m_pPositionBuffer->BufferSubData(0, m_particle.m_position);
}

}

#include "HalfEdgeStruct.h"
#include "BDB.h"
#include "Utility.h"
#include "GeometryUtility.h"
namespace KI
{

const char* const* HalfEdgeStruct::GetVertexValueString()
{
	static const char* parameter[] = {
		"None",
		"HeatValue",
		"VertexArea",
		"SDF",
		"MinCurvature",
		"MaxCurvature"
	};

	return parameter;
}

const char* const* HalfEdgeStruct::GetVertexDirectionString()
{
	static const char* parameter[] = {
		"None",
		"MinCurvature",
		"MaxCurvature",
	};

	return parameter;
}

String HalfEdgeStruct::ToString(HalfEdgeStruct::VertexValue param)
{
	return GetVertexValueString()[(int)param];
}

HalfEdgeStruct::Edge HalfEdgeStruct::GetEdge(int edgeIndex) const
{
	HalfEdgeStruct::Edge edge;
	edge.begin = m_position[m_halfEdge[m_halfEdge[edgeIndex].oppositeEdge].endPos];
	edge.end = m_position[m_halfEdge[edgeIndex].endPos];
	return edge;
}


HalfEdgeStruct::IndexedEdge HalfEdgeStruct::GetIndexedEdge(int edgeIndex) const
{
	HalfEdgeStruct::IndexedEdge edge;
	edge[0] = m_halfEdge[m_halfEdge[edgeIndex].oppositeEdge].endPos;
	edge[1] = m_halfEdge[edgeIndex].endPos;
	return edge;
}

const HalfEdgeStruct::IndexedFace& HalfEdgeStruct::GetIndexedFace(int faceIndex) const
{
	return m_face[faceIndex];
}

HalfEdgeStruct::Face HalfEdgeStruct::GetFace(int faceIndex) const
{
	auto index = GetIndexedFace(faceIndex);
	Face face;
	face.pos0 = m_position[index.position[0]];
	face.pos1 = m_position[index.position[1]];
	face.pos2 = m_position[index.position[2]];
	return face;
}

Vector3 HalfEdgeStruct::CalcGravity(int faceIndex) const
{
	return CalcGravity(GetIndexedFace(faceIndex));
}

float HalfEdgeStruct::CalcFaceArea(int faceIndex) const
{
	auto face = m_face[faceIndex];
	auto v0 = m_position[face.position[0]];
	auto v1 = m_position[face.position[1]];
	auto v2 = m_position[face.position[2]];

	return 0.5f * glm::length(glm::cross((v1 - v0), (v2 - v0)));

}
Vector3 HalfEdgeStruct::CalcGravity(const IndexedFace& face) const
{
	auto position = m_position[face.position[0]];
	position += m_position[face.position[1]];
	position += m_position[face.position[2]];
	position /= 3.0f;
	return position;
}

Vector3 HalfEdgeStruct::CalcFaceNormal(int faceIndex) const
{
	auto p0 = m_position[m_face[faceIndex].position[0]];
	auto p1 = m_position[m_face[faceIndex].position[1]];
	auto p2 = m_position[m_face[faceIndex].position[2]];

	auto v1 = p1 - p0;
	auto v2 = p2 - p0;

	return glm::normalize(glm::cross(v1, v2));
}

Vector<unsigned int> HalfEdgeStruct::GetAroundFace(const IndexedFace& face) const
{
	Vector<unsigned int> aroundFace(3);

	aroundFace[0] = m_halfEdge[m_halfEdge[face.edge[0]].oppositeEdge].face;
	aroundFace[1] = m_halfEdge[m_halfEdge[face.edge[1]].oppositeEdge].face;
	aroundFace[2] = m_halfEdge[m_halfEdge[face.edge[2]].oppositeEdge].face;

	return aroundFace;
}
Vector<int> HalfEdgeStruct::GetAroundEdge(int posIndex) const
{
	Vector<int> loopEdge;

	int edgeIndex = m_positionToEdge[posIndex];
	loopEdge.push_back(edgeIndex);
	while (true) {
		const auto& edge = m_halfEdge[edgeIndex];
		edgeIndex = m_halfEdge[edge.oppositeEdge].nextEdge;
		if (edgeIndex == loopEdge[0]) {
			break;
		}
		loopEdge.push_back(edgeIndex);
	}

	return loopEdge;
}

Vector<int> HalfEdgeStruct::GetAroundFaceFromPosition(int posIndex) const
{
	auto aroundEdge = GetAroundEdge(posIndex);
	Vector<int> aroundFace;
	for (auto edge : aroundEdge) {
		aroundFace.push_back(m_halfEdge[edge].face);
	}

	return aroundFace;
}
Vector<unsigned int> HalfEdgeStruct::CreateIndexBufferData() const
{
	Vector<unsigned int> indexBuffer;
	indexBuffer.resize(m_face.size() * 3);
	int counter = 0;
	for (const auto& face : m_face) {
		indexBuffer[counter++] = face.position[0];
		indexBuffer[counter++] = face.position[1];
		indexBuffer[counter++] = face.position[2];
	}

	return indexBuffer;
}

Vector<unsigned int> HalfEdgeStruct::CreateEdgeIndexBufferData()
{
	Vector<unsigned int> indexBuffer;
	indexBuffer.resize(m_halfEdge.size() * 2);
	for (size_t i = 0; i < m_halfEdge.size(); i++) {
		auto edge = GetIndexedEdge(i);
		indexBuffer[2 * i] = edge[0];
		indexBuffer[2 * i + 1] = edge[1];
	}

	return indexBuffer;
}

BDB HalfEdgeStruct::CreateBDB() const
{
	BDB bdb;
	for (const auto& pos : m_position) {
		bdb.Add(pos);
	}
	return bdb;
}

void HalfEdgeStruct::CreateFace()
{
	m_face.resize(m_faceToEdge.size());
	int counter = 0;
	for (auto edgeIndex : m_faceToEdge) {
		auto face = IndexedFace();
		const auto& edge = m_halfEdge[edgeIndex];
		face.edge[0] = edgeIndex;
		face.edge[1] = edge.beforeEdge;
		face.edge[2] = edge.nextEdge;

		face.position[0] = m_halfEdge[face.edge[0]].endPos;
		face.position[1] = m_halfEdge[face.edge[1]].endPos;
		face.position[2] = m_halfEdge[face.edge[2]].endPos;
		m_face[counter++] = std::move(face);
	}
}
const Vector<Vector3>& HalfEdgeStruct::GetNormal()
{
	if (m_parameter.vertexNormal.size() == 0) { CreateNormal(); }
	return m_parameter.vertexNormal;
}
Vector3 HalfEdgeStruct::GetNormal(int index)
{
	if (m_parameter.vertexNormal.size() == 0) {
		CreateNormal();
	}

	return m_parameter.vertexNormal[index];
}
void HalfEdgeStruct::CreateNormal()
{
	m_parameter.vertexNormal.resize(m_position.size());
	for (size_t i = 0; i < m_position.size(); i++) {
		auto faces = GetAroundFaceFromPosition(i);
		Vector3 normal(0);
		for (size_t j = 0; j < faces.size(); j++) {
			normal += CalcFaceNormal(faces[j]);
		}
		normal.x = normal.x / faces.size();
		normal.y = normal.y / faces.size();
		normal.z = normal.z / faces.size();
		m_parameter.vertexNormal[i] = glm::normalize(normal);
	}
}

Vector<Vector3> HalfEdgeStruct::ConvertVertexColorToFaceColor(const Vector<Vector3>& vertexColor) const
{
	Vector<Vector3> faceColor(GetFaceNum() * 3);
	for (size_t i = 0; i < GetFaceNum(); i++) {
		const auto& face = GetIndexedFace(i);
		faceColor[3 * i] = (vertexColor[face.position[0]]);
		faceColor[3 * i + 1] = (vertexColor[face.position[1]]);
		faceColor[3 * i + 2] = (vertexColor[face.position[2]]);
	}

	return faceColor;
}


float HalfEdgeStruct::CalcVertexArea(int position) const
{
	auto area = 0.0f;
	auto arondFace = GetAroundFaceFromPosition(position);
	for (auto faceIndex : arondFace) {
		area += CalcFaceArea(faceIndex) / 3.0f;
	}

	return area;
}

float CalcCotangentVal(const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
	auto u = v1 - v0;
	auto v = v2 - v0;

	auto cosVal = glm::dot(u, v);
	auto sinVal = glm::length(glm::cross(u, v));
	return cosVal / std::max(sinVal, 1e-8f);
}

void HalfEdgeStruct::CreateVertexArea()
{
	if (m_parameter.vertexArea.size() != 0) { return; }
	m_parameter.vertexArea.resize(GetVertexNum());
	for (int i = 0; i < m_parameter.vertexArea.size(); i++) {
		m_parameter.vertexArea[i] = CalcVertexArea(i);
	}
}

float HalfEdgeStruct::GetVertexArea(int position) const
{
	if (m_parameter.vertexArea.size() != 0) {
		return m_parameter.vertexArea[position];
	}

	return CalcVertexArea(position);
}
float HalfEdgeStruct::CalcCotangent(int edgeIndex) const
{
	auto currentHalf = GetHalfEdge(edgeIndex);
	auto currentEdge = GetEdge(edgeIndex);
	auto nextEdge = GetEdge(currentHalf.nextEdge);
	auto oppoNextEdge = GetEdge(GetHalfEdge(currentHalf.oppositeEdge).nextEdge);
	
	
	auto cotAlpha = CalcCotangentVal(nextEdge.end, currentEdge.begin, currentEdge.end);
	auto cotBeta = CalcCotangentVal(oppoNextEdge.end, currentEdge.end, currentEdge.begin);

	return (cotAlpha + cotBeta) * 0.5f;
}

void HalfEdgeStruct::CreateCotangentLaplasian()
{
	if (m_parameter.cotangent.size() != 0) { return; }
	std::vector<Eigen::Triplet<float>> cotTriplets;
	for (int i = 0; i < GetVertexNum(); i++) {
		auto aroundEdge = GetAroundEdge(i);
		auto sumWeight = 0.0f;
		for (int j = 0; j < aroundEdge.size(); j++) {
			auto currentHalf = GetHalfEdge(aroundEdge[j]);
			auto cotangent = CalcCotangent(aroundEdge[j]) / (GetVertexArea(i) * GetVertexArea(currentHalf.endPos));
			cotTriplets.emplace_back(Eigen::Triplet<float>(i, j, -cotangent));
			sumWeight += cotangent;
		}

		cotTriplets.emplace_back(Eigen::Triplet<float>(i, i, sumWeight));
	}

	m_parameter.cotangent.resize(GetVertexNum(), GetVertexNum());
	m_parameter.cotangent.setFromTriplets(cotTriplets.begin(), cotTriplets.end());
}

void HalfEdgeStruct::CreateHeatMethod(float timeStep, int position)
{
	if (m_parameter.heatValue.size() != 0) { return; }
	CreateCotangentLaplasian();
	Eigen::SparseMatrix<float> I(GetVertexNum(), GetVertexNum());
	I.setIdentity();
	auto A = I - timeStep * m_parameter.cotangent;
	Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;
	solver.compute(A);


	Eigen::VectorXf u0(GetVertexNum());
	u0.setZero();
	u0[position] = 1.0f;


	Eigen::VectorXf u = solver.solve(u0);
	float min = u[0];
	float max = u[0];
	for (int i = 1; i < u.size(); i++) {
		min = std::min(min, u[i]);
		max = std::max(max, u[i]);
	}

	m_parameter.heatValue.resize(GetVertexNum());
	for (int i = 0; i < GetVertexNum(); i++) {
		m_parameter.heatValue[i] = u[i];
	}
}

void HalfEdgeStruct::CreateDirectionField()
{
	if (m_parameter.minCurvature.size() != 0) { return; }
	// 1. 法線ベクトル n_v を使って、接平面基底 (t1, t2) を構築
	Vector3 t1, t2;
	m_parameter.minCurvature.resize(m_position.size());
	m_parameter.maxCurvature.resize(m_position.size());
	m_parameter.minDirection.resize(m_position.size());
	m_parameter.maxDirection.resize(m_position.size());
	for (int i = 0; i < m_position.size(); i++) {
		const auto& vi = m_position[i];
		const auto& normal = m_parameter.vertexNormal[i];
		GeometryUtility::CreateTangentBasis(normal, t1, t2);
		// 2. Shape operator S (2x2 対称行列) を初期化
		Eigen::Matrix2f S = Eigen::Matrix2f::Zero();
		auto aroundEdge = GetAroundEdge(i);
		float totalWeight = 0.0f;
		for (const auto& edgeIndex : aroundEdge) {
			const auto& edge = m_halfEdge[edgeIndex];
			const auto& vj = m_position[edge.endPos];
			const auto& normalJ = m_parameter.vertexNormal[edge.endPos];
			auto dir = vj - vi;
			auto dNormal = normalJ - normal;

			double weight_sum = 0.0;
			auto p0 = GetNextPos(edgeIndex);
			auto p1 = GetNextPos(m_halfEdge[edgeIndex].oppositeEdge);
			weight_sum += GeometryUtility::CalcCotangent(vi, p0, vj);
			weight_sum += GeometryUtility::CalcCotangent(vi, p1, vj);

			auto x = glm::dot(dir, t1);
			auto y = glm::dot(dir, t2);
			auto dx = glm::dot(dNormal, t1);
			auto dy = glm::dot(dNormal, t2);

			S(0, 0) += weight_sum * dx * x;
			S(0, 1) += weight_sum * dx * y;
			S(1, 0) += weight_sum * dy * x;
			S(1, 1) += weight_sum * dy * y;

			totalWeight += weight_sum * (x * x + y * y);
			/*
			// 3-1. Edge方向を接平面上に投影（pj_proj）
			auto pj_proj = dir - glm::dot(dir, normal) * normal;

			// 3-2. 法線差（接平面上への変化率）→ dN
			auto dN = normalJ - normal;

			// 3-3. pj_proj を接平面基底で表現（2D）
			double u = dot(pj_proj, t1);
			double v = dot(pj_proj, t2);

			// 3-4. dN を接平面基底で表現（2D）
			double dNu = dot(dN, t1);
			double dNv = dot(dN, t2);

			// 3-5. 重み（例：距離ベース、cot重みなど）を計算
			double weight = 1.0 / (glm::length(pj_proj) + 0.0001); // avoid div0

			// 3-6. Shape operator（近似ヤコビアン）を加算
			S(0, 0) += weight * dNu * u;
			S(0, 1) += weight * dNu * v;
			S(1, 0) += weight * dNv * u;
			S(1, 1) += weight * dNv * v;

			weight_sum += weight;
			*/
		}

		// 4. 正規化

		S /= totalWeight;

		// 5. 固有値・固有ベクトルを求める（Sは対称行列）
		Eigen::SelfAdjointEigenSolver<Eigen::Matrix2f> solver(S);
		if (solver.info() == Eigen::Success) {
			Eigen::Vector2f eigenvalues = solver.eigenvalues();      // 昇順
			Eigen::Matrix2f eigenvectors = solver.eigenvectors();    // 各列が固有ベクトル

			Eigen::Vector2f dir1 = eigenvectors.col(1); // k1 に対応する固有ベクトル
			Eigen::Vector2f dir2 = eigenvectors.col(0); // k2 に対応する固有ベクトル

			// 6. 接平面上の方向ベクトルを3Dに変換
			m_parameter.maxCurvature[i] = eigenvalues[1];
			m_parameter.minCurvature[i] = eigenvalues[0];
			m_parameter.minDirection[i] = glm::normalize(dir1.x() * t1 + dir1.y() * t2);
			m_parameter.maxDirection[i] = glm::normalize(dir2.x() * t1 + dir2.y() * t2);
		}
	}
}

void HalfEdgeStruct::AddVertexOnFace(int faceIndex)
{

}
void HalfEdgeStruct::AddVertexOnEdge(int edgeIndex)
{

}

const Vector3& HalfEdgeStruct::GetNextPos(int edgeIndex) const
{
	return m_position[m_halfEdge[m_halfEdge[edgeIndex].nextEdge].endPos];
}
}
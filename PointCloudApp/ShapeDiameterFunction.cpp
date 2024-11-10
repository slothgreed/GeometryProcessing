#include "ShapeDiameterFunction.h"
#include "HalfEdgeNode.h"
#include "PrimitiveNode.h"
#include "Utility.h"
#include "Primitives.h"
#include "Random.h"
#include <glm/gtx/rotate_vector.hpp>

namespace KI
{

std::vector<Vector3> CraeteSamplingCircle(glm::vec3 tip, glm::vec3 axis, float height, float radius, int segments)
{
	axis = glm::normalize(axis);  // 軸ベクトルを正規化

	// 底面の中心位置
	glm::vec3 baseCenter = tip + axis * height;

	// 底面の円周上の点を生成するための基準ベクトルを取得
	glm::vec3 orthoVec = glm::normalize(glm::cross(axis, glm::vec3(1.0f, 0.0f, 0.0f)));
	if (glm::length(orthoVec) < 0.001f) {
		orthoVec = glm::normalize(glm::cross(axis, glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	orthoVec *= radius;  // 半径を考慮したベクトルにスケーリング
	// 頂点として円錐の先端を追加
	std::vector<Vector3> vertices;
	// 円周上の点を追加
	for (int i = 0; i < segments; ++i) {
		float angle = glm::two_pi<float>() * i / segments;
		vertices.push_back(baseCenter + glm::rotate(orthoVec, angle, axis));
	}

	return vertices;
}

ShapeDiameterFunction::ShapeDiameterFunction(HalfEdgeNode* pNode)
	:m_pHalfEdge(pNode)
	,m_samplingNum(10)
	, m_debugIndex(-1)
{
}

ShapeDiameterFunction::~ShapeDiameterFunction()
{
}

void ShapeDiameterFunction::Execute()
{
	m_maxValue = 0;
	auto pHalfEdge = m_pHalfEdge->GetData();
	m_result.resize(pHalfEdge->GetPositionNum());
	for (int i = 0; i < pHalfEdge->GetPositionNum(); i++) {
		const auto& position = pHalfEdge->GetPosition()[i];
		const auto& normal = pHalfEdge->GetNormal(i);
		auto circle = CraeteSamplingCircle(position, -normal, 10, 1, 10);
		//float value = Gaccho::rnd(0, 99);
		float value = 0.0f;
		float intersectNum = 0;
		for (int k = 0; k < circle.size(); k++) {
			float minValue = std::numeric_limits<float>::max();
			Ray::IntersectResult min;
			for (int j = 0; j < pHalfEdge->GetFaceNum(); j++) {
				auto face = pHalfEdge->GetFace(j);
				auto result = Ray(position, glm::normalize(circle[k] - position)).Intersect(face.pos0, face.pos1, face.pos2, false);
				if (result.success) {
					if (result.distance < minValue) {
						minValue = result.distance;
						min = result;
					}
					intersectNum++;
				}
			}

			if (min.success) {
				value += min.distance;
			}
		}

		intersectNum = 1;
		m_result[i] = value / intersectNum;
		m_maxValue = m_result[i] < m_maxValue ? m_maxValue : m_result[i];
	}

}

Vector<Vector3> ShapeDiameterFunction::GetResultFaceColor() const
{
	return m_pHalfEdge->GetData()->ConvertVertexColorToFaceColor(GetResultVertexColor());
}
Vector<Vector3> ShapeDiameterFunction::GetResultVertexColor() const
{
	Vector<Vector3> vertexColor(m_result.size());
	for (int i = 0; i < m_result.size(); i++) {
		vertexColor[i] = ColorUtility::CreatePseudo(m_result[i], m_maxValue);
	}
	return vertexColor;
}
void ShapeDiameterFunction::ShowUI()
{
	if (ImGui::SliderInt("ShapeDiameterFunctionDebug",&m_debugIndex,-1,m_pHalfEdge->GetData()->GetPositionNum())) {
		if (m_debugIndex < 0) { return; }
		auto pRays = std::make_shared<Primitive>();
		Vector<Vector3> pos;
		for (int i = m_debugIndex; i < m_debugIndex + 1; i++) {
			auto pHalfEdge = m_pHalfEdge->GetData();
			const auto& position = pHalfEdge->GetPosition()[i];
			const auto& normal = pHalfEdge->GetNormal(i);
			auto circle = CraeteSamplingCircle(position, -normal, 10, 1, 10);
			float value = 0.0f;
			for (int k = 0; k < circle.size(); k++) {
				float minValue = std::numeric_limits<float>::max();
				Ray::IntersectResult min;
				Vector3 source;
				for (int j = 0; j < pHalfEdge->GetFaceNum(); j++) {
					auto face = pHalfEdge->GetFace(j);
					auto result = Ray(position, glm::normalize(circle[k] - position)).Intersect(face.pos0, face.pos1, face.pos2, false);
					if (result.success) {
						if (result.distance < minValue) {
							minValue = result.distance;
							min = result;
							source = position;
						}
					}
				}

				if (min.success) {
					pos.push_back(source);
					pos.push_back(min.position);
				} else {
					pos.push_back(position);
					pos.push_back(position + (circle[k] - position) * 100.0f);
				}
			}


		}
		pRays->SetPosition(std::move(pos));
		pRays->SetType(GL_LINES);
		auto pNode = std::make_shared<PrimitiveNode>("ShapeDiameterRay", pRays, ColorUtility::CreatePrimary(1));
		pNode->SetMatrix(m_pHalfEdge->GetMatrix());
		m_pHalfEdge->AddNode(pNode);
	}
}
}



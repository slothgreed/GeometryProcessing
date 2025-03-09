#include "SignedDistanceField.h"
#include "HalfEdgeNode.h"
#include "Primitives.h"
#include "PrimitiveNode.h"
namespace KI
{
SignedDistanceField::SignedDistanceField(HalfEdgeNode* pHalfEdge)
	:m_pHalfEdge(pHalfEdge)
{
}

SignedDistanceField::~SignedDistanceField()
{
}

void SignedDistanceField::Execute()
{

}

SignedDistanceField::UI::UI()
{
	glStatus = std::make_shared<GLStatus>();
	glStatus->SetBackCull(false);
}
void SignedDistanceField::ShowUI()
{
	ImGui::Checkbox("VisibleX", &m_ui.xPlane.visible);
	const auto& bdb = m_pHalfEdge->GetBoundBox();
	if (m_ui.xPlane.visible) {
		if (ImGui::SliderFloat("XPlane", &m_ui.xPlane.position, bdb.Min().x, bdb.Max().x, "%lf", 1.0f)) {
			std::shared_ptr<Primitive> pPlane = std::make_shared<Plane>(bdb.Min(), bdb.Max(), m_ui.xPlane.position, Plane::X);
			auto pNode = std::make_shared<PrimitiveNode>("XPlane", pPlane, Vector3(1.0, 0.0, 0.0));
			pNode->SetGLStatus(m_ui.glStatus);
			m_pHalfEdge->AddNode(pNode);
		}
	} else {
		m_pHalfEdge->RemoveNode("XPlane");
	}

	ImGui::Checkbox("VisibleY", &m_ui.yPlane.visible);
	if (m_ui.yPlane.visible) {
		if (ImGui::SliderFloat("YPlane", &m_ui.yPlane.position, bdb.Min().y, bdb.Max().y, "%lf", 1.0f)) {
			std::shared_ptr<Primitive> pPlane = std::make_shared<Plane>(bdb.Min(), bdb.Max(), m_ui.yPlane.position, Plane::Y);
			auto pNode = std::make_shared<PrimitiveNode>("YPlane", pPlane, Vector3(0.0, 1.0, 0.0));
			pNode->SetGLStatus(m_ui.glStatus);
			m_pHalfEdge->AddNode(pNode);
		}
	} else {
		m_pHalfEdge->RemoveNode("YPlane");
	}

	ImGui::Checkbox("VisibleZ", &m_ui.zPlane.visible);
	if (m_ui.zPlane.visible) {
		if (ImGui::SliderFloat("ZPlane", &m_ui.zPlane.position, bdb.Min().z, bdb.Max().z, "%lf", 1.0f)) {
			std::shared_ptr<Primitive> pPlane = std::make_shared<Plane>(bdb.Min(), bdb.Max(), m_ui.zPlane.position, Plane::Z);
			auto pNode = std::make_shared<PrimitiveNode>("ZPlane", pPlane, Vector3(0.0, 0.0, 1.0));
			pNode->SetGLStatus(m_ui.glStatus);
			m_pHalfEdge->AddNode(pNode);
		}
	} else {
		m_pHalfEdge->RemoveNode("ZPlane");
	}


}
}

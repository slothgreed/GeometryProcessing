#include "HalfEdgeController.h"
#include "Utility.h"
namespace KI
{

bool HalfEdgeController::Move(const EditContext& context)
{
	if (!m_pParts) { return false; }
	
	if (context.GetMouse()->Press(MOUSE_BUTTON_LEFT)) {
		if (m_pParts->IsVertex()) {
			MoveVertex(context, m_pParts->parts);
		}
	}

	return true;
}

void HalfEdgeController::SetTarget(const Shared<HalfEdgeNode::HalfEdgeParts>& pParts)
{
	if (m_pParts == pParts) { return; }
	m_pParts = pParts;
}


void HalfEdgeController::MoveVertex(const EditContext& context, int vertexIndex)
{
	if (!m_pNode) { return; }
	auto position = m_pNode->GetData()->GetVertex(vertexIndex);
	position = MathHelper::Transform(m_pNode->GetMatrix(), position);
	auto plane = Plane(position, context.GetCamera()->Direction());
	auto ray = context.GetCamera()->CreateRay(context.GetMouse()->Position());
	auto intersect = ray.Intersect(plane);
	if (!intersect.success) { return; }

	auto invModel = glm::inverse(m_pNode->GetMatrix());
	auto newPos = MathHelper::Transform(invModel, intersect.position);
	m_pNode->GetData()->SetVertex(vertexIndex, newPos);
	m_pNode->UpdateVertex();
	DebugPrintf::Int("Move", vertexIndex);
	DebugPrintf::Vec3(":", newPos); DebugPrintf::NewLine();
}

}
#ifndef HALFEDGE_CONTROLLER_H
#define HALFEDGE_CONTROLLER_H
#include "MouseController.h"
#include "HalfEdgeNode.h"
namespace KI
{
class HalfEdgeNode;
class HalfEdgeController : public MouseController
{
public:
	HalfEdgeController(HalfEdgeNode* pNode)
		:m_pNode(pNode)	{};
	virtual ~HalfEdgeController() {};

	void SetTarget(const Shared<HalfEdgeNode::HalfEdgeParts>& pParts);
	virtual bool Move(const EditContext& mouse);

private:
	void MoveVertex(const EditContext& context, int vertexIndex);
	HalfEdgeNode* m_pNode;
	Shared<HalfEdgeNode::HalfEdgeParts> m_pParts;
};
}

#endif HALFEDGE_CONTROLLER_H
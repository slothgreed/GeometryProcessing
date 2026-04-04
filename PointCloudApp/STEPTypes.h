#ifndef KI_STEP_TYPES_H
#define KI_STEP_TYPES_H
namespace KI
{
enum STEPEnum
{
	DOLL = -1,
	ASTERISK = -2,
};
struct STEPLine;
struct STEPPlane;
struct STEPInterSectionCurve;
struct STEPVector;
struct STEPDirection;
struct STEPPoint;
struct STEPAxis2Placement3D;
struct STEPEdgeCurve;
struct STEPVertexPoint;
struct STEPEdgeLoop;
struct STEPPolyLoop;
struct STEPFaceOuterBound;
struct STEPFaceBound;
struct STEPOrientedEdge;
struct STEPAdvancedFace;
struct STEPFaceSurface;
struct STEPClosedShell;
struct STEPOpenShell;
struct STEPCircle;
struct STEPCylinderSurface;

enum ESTEPEntityType
{
	ESTEPLine,
	ESTEPPlane,
	ESTEPInterSectionCurve,
	ESTEPVector,
	ESTEPDirection,
	ESTEPPoint,
	ESTEPAxis2Placement3D,
	ESTEPEdgeCurve,
	ESTEPVertexPoint,
	ESTEPEdgeLoop,
	ESTEPPolyLoop,
	ESTEPFaceOuterBound,
	ESTEPFaceBound,
	ESTEPOrientedEdge,
	ESTEPAdvancedFace,
	ESTEPFaceSurface,
	ESTEPClosedShell,
	ESTEPOpenShell,
	ESTEPCircle,
	ESTEPCylinderSurface,
};
struct STEPStruct;
struct STEPEntityBase;
class STEPRenderNode;
struct UIContext;
struct STEPUIContext
{
	bool IsSelect(int id) const;
	int GetSelectId() const;
	STEPRenderNode* pNode = nullptr;
	STEPEntityBase* pSelect = nullptr;
	UIContext* ui = nullptr;
};

}

#endif KI_STEP_TYPES_H

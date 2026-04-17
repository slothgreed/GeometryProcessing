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
struct STEPCylindricalSurface;
struct STEPConicalSurface;
struct STEPToroidalSurface;
struct STEPQuasiUniformCurve;
struct STEPBSplineCurve;
struct STEPBSplineSurface;
struct STEPBSplineSurfaceWithKnots;
enum ESTEPEntityType
{
	ESTEPNone,
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
	ESTEPCylindricalSurface,
	ESTEPConicalSurface,
	ESTEPToroidalSurface,
	ESTEPQuasiUniformCurve,
	ESTEPBSplineCurve,
	ESTEPBSplineCurveWithNot,
	ESTEPRationalBSplineCurve,
	ESTEPBSplineSurface,
	ESTEPBSplineSurfaceWithKnots
};

enum class STEPBSplineCurveFormType
{
	POLYLINE,
	CIRCULAR_ARC,
	ELLIPTIC_ARC,
	PARABOLIC_ARC,
	HYPERBOLIC_ARC,
	UNSPECIFIED

};
enum class STEPLogicalType
{
	True,
	False,
	UNDEFINED
};

enum class STEPKnotType
{
	UNIFORM_KNOTS,
	QUASI_UNIFORM_KNOTS,
	PIECEWISE_BEZIER_KNOTS,
	UNSPECIFIED
};

enum class STEPBSplineSurfaceFormType
{
	PLANE_SURF,
	CYLINDRICAL_SURF,
	CONICAL_SURF,
	SPHERICAL_SURF,
	TOROIDAL_SURF,
	SURF_OF_REVOLUTION,
	RULED_SURF,
	GENERALISED_CONE,
	QUADRIC_SURF,
	SURF_OF_LINEAR_EXTRUSION,
	UNSPECIFIED
};
template<typename T>
struct STEPUV
{
	T u;
	T v;
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
	String filePath;
};

}

#endif KI_STEP_TYPES_H

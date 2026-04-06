#ifndef KI_STEP_ENTITY_H
#define KI_STEP_ENTITY_H
#include "STEPTypes.h"
#include "FileUtility.h"
#include "Utility.h"
#include "Polyline.h"
#include "Mesh.h"
namespace KI
{

#define FIND_SET_DATA(a,b,c,d) { auto x = step.c.find(d); if(x != step.c.end()) { a.b = x->second->ToData(step);}}
#define FIND_SET_DATA2(a,b,c) { auto x = step.b.find(c); if(x == step.b.end()) {a = x->second->ToData(step);}}


const int CIRCLE_SUBDIVISION_NUM = 36;
#define STEP_DEFINE_CAST(TypeName) \
static TypeName* Cast(STEPEntityBase* pBase) \
{ \
    if (!pBase) { return nullptr; } \
    return  (pBase->GetType() == TypeName::ClassType) ? static_cast<TypeName*>(pBase) : nullptr; \
} \
\
static const TypeName* Cast(const STEPEntityBase* pBase) \
{ \
    if (!pBase) { return nullptr; } \
    return (pBase->GetType() == TypeName::ClassType) ? static_cast<const TypeName*>(pBase) : nullptr; \
}\

// 1段目（結合専用）
#define MACRO_CONCAT(a, b) a##b
// 2段目（展開させてから結合）
#define MACRO_CONCAT_EXPAND(a, b) MACRO_CONCAT(a, b)

#define STEP_DEFINE_HPP(TypeName,_EntityName) \
static constexpr const char* EntityName = _EntityName;\
static constexpr ESTEPEntityType ClassType = MACRO_CONCAT_EXPAND(E,TypeName);\
virtual ESTEPEntityType GetType() const { return MACRO_CONCAT_EXPAND(E,TypeName); }\
static TypeName* Cast(STEPEntityBase* pBase) \
{ \
    if (!pBase) { return nullptr; } \
    return  (pBase->GetType() == TypeName::ClassType) ? static_cast<TypeName*>(pBase) : nullptr; \
} \
\
static const TypeName* Cast(const STEPEntityBase* pBase) \
{ \
    if (!pBase) { return nullptr; } \
    return (pBase->GetType() == TypeName::ClassType) ? static_cast<const TypeName*>(pBase) : nullptr; \
}\

struct DebugOption
{
	int level = -1;
};

struct STEPShape
{
	Vector<std::pair<int, Mesh>> meshs;
	Vector<PolylineList> polylineList;
	void AddMesh(int key, Mesh&& value)
	{
		if (value.TriangleNum() != 0) meshs.push_back(std::pair<int, Mesh>(key, std::move(value)));
	}
	void AddPolyline(PolylineList&& value)
	{
		if (value.Num() != 0) polylineList.push_back(std::move(value));
	}
	BDB CreateBDB() const
	{
		BDB bdb;
		for (size_t i = 0; i < meshs.size(); i++) { bdb.Add(BDB(meshs[i].second.GetPoints())); }
		for (size_t i = 0; i < polylineList.size(); i++) { bdb.Add(polylineList[i].CreateBDB()); }
		return bdb;
	}
};



struct STEPString
{
	STEPString() :id(-1) {};
	~STEPString() {};
	int id;
	String value;
	String name;
	static STEPString Create(const String& str);
	static Vector<String> SplitValue(const String& value);
	String CreateRemoveLabelStr() const;
	static String RemoveBracket(const String& str);
	static bool ValueToRef(const String& str, int& value);
	static bool ValueToFloat(const String& str, float& value);
	static bool IsAsterisk(const String& str);
	// 文字がドル記号か判定
	static bool IsDollar(const String& str);
	static bool ValueToBool(const String& str, bool& value);
	String ToString() const;
};

struct STEPStruct
{
	std::unordered_map<int, STEPPoint*> points;
	std::unordered_map<int, STEPLine*> lines;
	std::unordered_map<int, STEPCircle*> circles;
	std::unordered_map<int, STEPCylindricalSurface*> cylindricalSurface;
	std::unordered_map<int, STEPConicalSurface*> conicalSurface;
	std::unordered_map<int, STEPPlane*> planes;
	std::unordered_map<int, STEPVector*> vectors;
	std::unordered_map<int, STEPDirection*> directions;
	std::unordered_map<int, STEPEdgeCurve*> edgeCurve;
	std::unordered_map<int, STEPAxis2Placement3D*> axis2Placement3D;
	std::unordered_map<int, STEPVertexPoint*> vertexPoint;
	std::unordered_map<int, STEPInterSectionCurve*> interSectionCurve;
	std::unordered_map<int, STEPEdgeLoop*> edgeLoop;
	std::unordered_map<int, STEPPolyLoop*> polyLoop;
	std::unordered_map<int, STEPFaceOuterBound*> faceOuterBound;
	std::unordered_map<int, STEPFaceBound*> faceBound;
	std::unordered_map<int, STEPOrientedEdge*> orientedEdge;
	std::unordered_map<int, STEPAdvancedFace*> advancedFace;
	std::unordered_map<int, STEPFaceSurface*> faceSurface;
	std::unordered_map<int, STEPClosedShell*> closedShell;
	std::unordered_map<int, STEPOpenShell*> openShell;

	~STEPStruct();
};

template <typename Struct>
Struct* FindSetData2(const STEPStruct& step, const std::unordered_map<int, Struct*>& container, int key)
{
	auto it = container.find(key);
	if (it == container.end()) {
		DebugPrintf::Int("#=", key);
		DebugPrintf::StringStr(", NotFound", Struct::EntityName); DebugPrintf::NewLine();
		return nullptr;
	}
	it->second->FetchData(step);
	return it->second;
}

struct STEPEntityBase
{
	virtual ~STEPEntityBase() = default;
	int id = -1;
	String str;

	static void Fetch(STEPEntityBase* data, const STEPString& stepStr);
	virtual ESTEPEntityType GetType() const = 0;
	virtual void ShowUI(STEPUIContext& ui) = 0;
	bool ShowBranch(STEPUIContext& ui, bool select);
	virtual void Printf(const DebugOption& option) = 0;
	void ShowLeaf(STEPUIContext& ui);
	void PrintfRaw();
	String ToString() const;
};

struct STEPPoint : public STEPEntityBase
{
	virtual ~STEPPoint() = default;
	STEP_DEFINE_HPP(STEPPoint, "CARTESIAN_POINT");

	struct Data
	{
		Vector3 pos;
	};
	
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);

};

struct STEPDirection : public STEPEntityBase
{
	virtual ~STEPDirection() = default;
	STEP_DEFINE_HPP(STEPDirection, "DIRECTION");

	Vector3 direction;
	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPVector : public STEPEntityBase
{
	virtual ~STEPVector() = default;
	STEP_DEFINE_HPP(STEPVector, "VECTOR");

	struct Raw
	{
		int idRef = -1;
		float length = 0.0f;
	};

	Raw raw;
	struct Data
	{
		STEPDirection* driection = nullptr;
		Vector3 vector;
	};

	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);

};

struct STEPLine : public STEPEntityBase
{
	virtual ~STEPLine() = default;
	STEP_DEFINE_HPP(STEPLine, "LINE");

	struct Raw
	{
		int beginRef = -1;
		int vectorRef = -1;
	};

	Raw raw;
	struct Data
	{
		Vector3 begin;
		Vector3 vector;

		STEPPoint* point = nullptr;
		STEPVector* vector0 = nullptr;
		Polyline CreatePolyline();
	};
	Data data;
	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);

};

struct STEPAxis2Placement3D : public STEPEntityBase
{
	virtual ~STEPAxis2Placement3D() = default;
	STEP_DEFINE_HPP(STEPAxis2Placement3D, "AXIS2_PLACEMENT_3D");

	struct Raw
	{
		int pointRef = -1;
		int dirRef1 = -1;
		int dirRef2 = -1;
	};

	Raw raw;
	struct Data
	{
		Vector3 point;
		Vector3 dir1;
		Vector3 dir2;

		STEPPoint* point0 = nullptr;
		STEPDirection* direction1 = nullptr;
		STEPDirection* direction2 = nullptr;

		Vector3 Normal() const { return dir1; }
		Vector3 U() const { return dir2; }
		Vector3 V() const { return glm::cross(dir1, dir2); }
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};


struct STEPCircle : public STEPEntityBase
{
	virtual ~STEPCircle() = default;
	STEP_DEFINE_HPP(STEPCircle, "CIRCLE");
		
	struct Raw
	{
		int axisRef = -1;
		float rad = 0.0f;
	};
	Raw raw;

	struct Data
	{
		STEPAxis2Placement3D* axis = nullptr;
		float rad = 0.0f;

		Polyline CreatePolyline() const;
		Polyline CreatePolyline(const Vector3& begin, const Vector3& end) const;
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPCylindricalSurface : public STEPEntityBase
{
	virtual ~STEPCylindricalSurface() = default;
	STEP_DEFINE_HPP(STEPCylindricalSurface, "CYLINDRICAL_SURFACE");

	struct Raw
	{
		int axisRef = -1;
		float rad = 0.0f;
	};
	Raw raw;

	struct Data
	{
		STEPAxis2Placement3D* axis = nullptr;
		float rad = 0.0f;
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};


struct STEPConicalSurface : public STEPEntityBase
{
	virtual ~STEPConicalSurface() = default;
	STEP_DEFINE_HPP(STEPConicalSurface,"CONICAL_SURFACE")
	struct Raw
	{
		int idRef = 0;
	};

	Raw raw;
	struct Data
	{
		STEPAxis2Placement3D* axis = nullptr;
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPPlane : public STEPEntityBase
{
	virtual ~STEPPlane() = default;
	STEP_DEFINE_HPP(STEPPlane, "PLANE");

	struct Raw
	{
		int idRef = 0;
	};

	Raw raw;
	struct Data
	{
		STEPAxis2Placement3D* axis = nullptr;
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};


struct STEPInterSectionCurve : public STEPEntityBase
{
	virtual ~STEPInterSectionCurve() = default;
	STEP_DEFINE_HPP(STEPInterSectionCurve, "INTERSECTION_CURVE");

	struct Raw
	{
		int curveId = 0;
		int geomId0 = 0;
		int geomId1 = 0;
	};

	Raw raw;
	struct Data
	{
		enum class Type
		{
			None,
			Curve3D,
		};
		struct Curve
		{
			bool IsActive() const { return pLine || pCircle; }
			STEPLine* pLine = nullptr;
			STEPCircle* pCircle = nullptr;
		};
		Curve curve0;

		struct Surface
		{
			bool IsActive() const { return pPlane || pCylinderSurface; }
			STEPPlane* pPlane = nullptr;
			STEPCylindricalSurface* pCylinderSurface = nullptr;
		};
		Surface surf0;
		Surface surf1;
		Type type = Type::None;

		Polyline CreatePolyline(const Vector3& begin, const Vector3& end) const;
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPVertexPoint : public STEPEntityBase
{
	virtual ~STEPVertexPoint() = default;
	STEP_DEFINE_HPP(STEPVertexPoint, "VERTEX_POINT");

	struct Raw
	{
		int idRef = -1;
	};

	Raw raw;
	struct Data
	{
		STEPPoint* point = nullptr;
	};
	Data data;
	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPEdgeCurve : public STEPEntityBase
{
	virtual ~STEPEdgeCurve() = default;
	STEP_DEFINE_HPP(STEPEdgeCurve, "EDGE_CURVE");

	struct Raw
	{
		int vertRef0 = -1;
		int vertRef1 = -1;
		int lineRef2 = -1;
		bool sameSense = true;
	};

	Raw raw;

	struct Data
	{
		Vector3 begin;
		Vector3 end;
		STEPVertexPoint* pPoint0 = nullptr;
		STEPVertexPoint* pPoint1 = nullptr;
		STEPLine* line = nullptr;
		STEPCircle* circle = nullptr;
		STEPInterSectionCurve* intersectionCurve = nullptr;
		bool sameSense = true;

		Vector3 GetBegin() const { return sameSense ? begin : end; }
		Vector3 GetEnd() const { return sameSense ? end : begin; }
	};

	Polyline CreatePolyline() const;

	Data data;
	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPOrientedEdge : public STEPEntityBase
{
	virtual ~STEPOrientedEdge() = default;
	STEP_DEFINE_HPP(STEPOrientedEdge, "ORIENTED_EDGE");

	struct Raw
	{
		int vertRef0 = -1;
		int vertRef1 = -1;
		int edgeCurveRef2 = -1;
		bool orient = true;
	};

	Raw raw;

	struct Data
	{
		Vector3 begin;
		Vector3 end;
		STEPVertexPoint* vertex0 = nullptr;
		STEPVertexPoint* vertex1 = nullptr;
		STEPEdgeCurve* edgeCurve = nullptr;
		bool orient = true;

		bool IsLine() const { return edgeCurve->data.line != nullptr; }
		bool IsCircle() const { return edgeCurve->data.circle != nullptr; }
		Vector3 GetBegin() const { if (orient) { return begin; } else { return end; } }
		Vector3 GetEnd() const { if (orient) { return end; } else { return begin; } }
		float Length() const { return glm::length(GetBegin() - GetEnd()); }
		Vector3 Dir() const { return GetEnd() - GetBegin(); }
		Polyline CreatePolyline() const;
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);

};

struct STEPPolyLoop : public STEPEntityBase
{
	virtual ~STEPPolyLoop() = default;
	STEP_DEFINE_HPP(STEPPolyLoop, "POLY_LOOP");

	struct Raw
	{
		Vector<int> idRef;
	};
	Raw raw;

	struct Data
	{
		Vector<STEPPoint*> points;
		PolylineList CreatePolyline(int id) const;
	};

	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};


struct STEPEdgeLoop : public STEPEntityBase
{
	virtual ~STEPEdgeLoop() = default;
	STEP_DEFINE_HPP(STEPEdgeLoop, "EDGE_LOOP");

	struct Raw
	{
		Vector<int> idRef;
	};

	Raw raw;

	struct Data
	{
		Vector<STEPOrientedEdge*> orientedEdges;
		PolylineList CreatePolyline() const;
	};

	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPFaceBoundBase : public STEPEntityBase
{
	virtual ~STEPFaceBoundBase() = default;

	struct Raw
	{
		int idRef0 = -1;
		bool orient = true;
	};

	Raw raw;

	struct Data
	{
		STEPPolyLoop* polyLoop = nullptr;
		STEPEdgeLoop* edgeLoop = nullptr;
		bool orient = true;

		PolylineList CreatePolyline() const;
	};

	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr, STEPFaceBoundBase* data);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPFaceOuterBound : public STEPFaceBoundBase
{
	virtual ~STEPFaceOuterBound() = default;
	STEP_DEFINE_HPP(STEPFaceOuterBound, "FACE_OUTER_BOUND");

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
};

struct STEPFaceBound : public STEPFaceBoundBase
{
	virtual ~STEPFaceBound() = default;
	STEP_DEFINE_HPP(STEPFaceBound, "FACE_BOUND");

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
};

struct STEPFaceBase : public STEPEntityBase
{
	virtual ~STEPFaceBase() = default;
	struct Raw
	{
		Vector<int> faceRef0;
		int geomRef1 = -1;
		bool orient = true;
	};

	enum class GeomType
	{
		Plane,
		Cylinder,
	};

	struct Data
	{
		Vector<STEPFaceBound*> faceBound;
		Vector<STEPFaceOuterBound*> faceOuterBound;
		STEPPlane* plane = nullptr;
		STEPCylindricalSurface* cylinder = nullptr;
		STEPConicalSurface* conical = nullptr;
		bool orient = true;

		struct CylidnerEdge
		{
			bool IsActive() const { return true; }
			STEPOrientedEdge::Data* circle = nullptr;
			std::pair<float, Vector3> maxZ{ -INFINITY,Vector3(0, 0, 0) };
			std::pair<float, Vector3> minZ{ INFINITY, Vector3(0, 0, 0) };
			Vector3 begin = Vector3(0, 0, 0);
			Vector3 end = Vector3(0, 0, 0);
			float GetHeight() const { return maxZ.first - minZ.first; }
		};

		CylidnerEdge SearchCylinderEdge(const STEPCylindricalSurface* pCylinder) const;
		PolylineList CreateBoundPolyline() const;
		PolylineList CreateOuterBoundPolyline() const;
		Mesh CreateMesh(const Polyline& bound, const Polyline& outerBound) const;
	};

	void FetchData(const STEPStruct& step);

	Raw raw;
	Data data;

	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPFaceSurface : public STEPFaceBase
{
	virtual ~STEPFaceSurface() = default;
	STEP_DEFINE_HPP(STEPFaceSurface, "FACE_SURFACE");

	static void Fetch(STEPStruct& step, const STEPString& stepStr);

};

struct STEPAdvancedFace : public STEPFaceBase
{
	virtual ~STEPAdvancedFace() = default;
	STEP_DEFINE_HPP(STEPAdvancedFace, "ADVANCED_FACE");

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
};

struct STEPShell : public STEPEntityBase
{
	virtual ~STEPShell() = default;
	struct Raw
	{
		Vector<int> faceRef;
	};

	Raw raw;
	struct Data
	{
		Vector<STEPFaceSurface*> faceSurface;
		Vector<STEPAdvancedFace*> advancedFace;
		void Clear() { faceSurface.clear(); advancedFace.clear(); }
	};

	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr, STEPShell* pShell);
	void FetchData(const STEPStruct& step, STEPShell::Data* data);
	void CreateMesh(const STEPShell& step, STEPShape& shape);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);

};
struct STEPClosedShell : public STEPShell
{
	virtual ~STEPClosedShell() = default;
	STEP_DEFINE_HPP(STEPClosedShell, "CLOSED_SHELL");

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	STEPShape CreateMesh(const STEPStruct& step);

};

struct STEPOpenShell : public STEPShell
{
	virtual ~STEPOpenShell() = default;
	STEP_DEFINE_HPP(STEPOpenShell, "OPEN_SHELL");

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	STEPShape CreateMesh(const STEPStruct& step);
};
}

#endif KI_STEP_ENTITY_H
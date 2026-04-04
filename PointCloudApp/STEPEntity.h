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
	// ï∂éöÇ™ÉhÉããLçÜÇ©îªíË
	static bool IsDollar(const String& str);
	static bool ValueToBool(const String& str, bool& value);
	String ToString() const;
};

struct STEPStruct
{
	std::unordered_map<int, STEPPoint*> points;
	std::unordered_map<int, STEPLine*> lines;
	std::unordered_map<int, STEPCircle*> circles;
	std::unordered_map<int, STEPCylinderSurface*> cylinderSurface;
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

	~STEPStruct()
	{
		for (auto& v : points) { delete v.second; }
		for (auto& v : lines) { delete v.second; }
		for (auto& v : circles) { delete v.second; }
		for (auto& v : planes) { delete v.second; }
		for (auto& v : vectors) { delete v.second; }
		for (auto& v : directions) { delete v.second; }
		for (auto& v : edgeCurve) { delete v.second; }
		for (auto& v : axis2Placement3D) { delete v.second; }
		for (auto& v : cylinderSurface) { delete v.second; }
		for (auto& v : vertexPoint) { delete v.second; }
		for (auto& v : interSectionCurve) { delete v.second; }
		for (auto& v : edgeLoop) { delete v.second; }
		for (auto& v : polyLoop) { delete v.second; }
		for (auto& v : faceOuterBound) { delete v.second; }
		for (auto& v : faceBound) { delete v.second; }
		for (auto& v : orientedEdge) { delete v.second; }
		for (auto& v : advancedFace) { delete v.second; }
		for (auto& v : faceSurface) { delete v.second; }
		for (auto& v : closedShell) { delete v.second; }
		for (auto& v : openShell) { delete v.second; }
	}

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
};

struct STEPPoint : public STEPEntityBase
{
	virtual ~STEPPoint() = default;
	static constexpr const char* EntityName = "CARTESIAN_POINT";
	static constexpr ESTEPEntityType ClassType = ESTEPPoint;
	virtual ESTEPEntityType GetType() const { return ESTEPPoint; }
	STEP_DEFINE_CAST(STEPPoint)

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
	static constexpr const char* EntityName = "DIRECTION";
	static constexpr ESTEPEntityType ClassType = ESTEPDirection;
	virtual ESTEPEntityType GetType() const { return ESTEPDirection; }
	STEP_DEFINE_CAST(STEPDirection)

	Vector3 direction;
	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	void Printf(const DebugOption& option);
	void ShowUI(STEPUIContext& ui);
};

struct STEPVector : public STEPEntityBase
{
	virtual ~STEPVector() = default;
	static constexpr const char* EntityName = "VECTOR";
	static constexpr ESTEPEntityType ClassType = ESTEPVector;
	virtual ESTEPEntityType GetType() const { return ESTEPVector; }
	STEP_DEFINE_CAST(STEPVector)

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
	static constexpr const char* EntityName = "LINE";
	static constexpr ESTEPEntityType ClassType = ESTEPLine;
	virtual ESTEPEntityType GetType() const { return ESTEPLine; }
	STEP_DEFINE_CAST(STEPLine)

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
	static constexpr const char* EntityName = "AXIS2_PLACEMENT_3D";
	static constexpr ESTEPEntityType ClassType = ESTEPAxis2Placement3D;
	virtual ESTEPEntityType GetType() const { return ESTEPAxis2Placement3D; }
	STEP_DEFINE_CAST(STEPAxis2Placement3D)

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
	static constexpr const char* EntityName = "CIRCLE";
	static constexpr ESTEPEntityType ClassType = ESTEPCircle;
	virtual ESTEPEntityType GetType() const { return ESTEPCircle; }
	STEP_DEFINE_CAST(STEPCircle)
		
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

struct STEPCylinderSurface : public STEPEntityBase
{
	virtual ~STEPCylinderSurface() = default;
	static constexpr const char* EntityName = "CYLINDRICAL_SURFACE";
	static constexpr ESTEPEntityType ClassType = ESTEPCylinderSurface;
	virtual ESTEPEntityType GetType() const { return ESTEPCylinderSurface; }
	STEP_DEFINE_CAST(STEPCylinderSurface)

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


struct STEPPlane : public STEPEntityBase
{
	virtual ~STEPPlane() = default;
	static constexpr const char* EntityName = "PLANE";
	static constexpr ESTEPEntityType ClassType = ESTEPPlane;
	virtual ESTEPEntityType GetType() const { return ESTEPPlane; }
	STEP_DEFINE_CAST(STEPPlane)

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
	static constexpr const char* EntityName = "INTERSECTION_CURVE";
	static constexpr ESTEPEntityType ClassType = ESTEPInterSectionCurve;
	virtual ESTEPEntityType GetType() const { return ESTEPInterSectionCurve; }
	STEP_DEFINE_CAST(STEPInterSectionCurve)

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
			STEPCylinderSurface* pCylinderSurface = nullptr;
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
	static constexpr const char* EntityName = "VERTEX_POINT";
	static constexpr ESTEPEntityType ClassType = ESTEPVertexPoint;
	virtual ESTEPEntityType GetType() const { return ESTEPVertexPoint; }
	STEP_DEFINE_CAST(STEPVertexPoint)

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
	static constexpr const char* EntityName = "EDGE_CURVE";
	static constexpr ESTEPEntityType ClassType = ESTEPEdgeCurve;
	virtual ESTEPEntityType GetType() const { return ESTEPEdgeCurve; }
	STEP_DEFINE_CAST(STEPEdgeCurve)

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
	static constexpr const char* EntityName = "ORIENTED_EDGE";
	static constexpr ESTEPEntityType ClassType = ESTEPOrientedEdge;
	virtual ESTEPEntityType GetType() const { return ESTEPOrientedEdge; }
	STEP_DEFINE_CAST(STEPOrientedEdge)

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
	static constexpr const char* EntityName = "POLY_LOOP";
	static constexpr ESTEPEntityType ClassType = ESTEPPolyLoop;
	virtual ESTEPEntityType GetType() const { return ESTEPPolyLoop; }
	STEP_DEFINE_CAST(STEPPolyLoop)

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
	static constexpr const char* EntityName = "EDGE_LOOP";
	static constexpr ESTEPEntityType ClassType = ESTEPEdgeLoop;
	virtual ESTEPEntityType GetType() const { return ESTEPEdgeLoop; }
	STEP_DEFINE_CAST(STEPEdgeLoop)

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
	static constexpr const char* EntityName = "FACE_OUTER_BOUND";
	static constexpr ESTEPEntityType ClassType = ESTEPFaceOuterBound;
	virtual ESTEPEntityType GetType() const { return ESTEPFaceOuterBound; }
	STEP_DEFINE_CAST(STEPFaceOuterBound)

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
};

struct STEPFaceBound : public STEPFaceBoundBase
{
	virtual ~STEPFaceBound() = default;
	static constexpr const char* EntityName = "FACE_BOUND";
	static constexpr ESTEPEntityType ClassType = ESTEPFaceBound;
	virtual ESTEPEntityType GetType() const { return ESTEPFaceBound; }
	STEP_DEFINE_CAST(STEPFaceBound)

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
		STEPCylinderSurface* cylinder = nullptr;
		bool orient = true;

		struct CylidnerEdge
		{
			bool IsActive() const { return circle; }
			STEPOrientedEdge::Data* circle = nullptr;
			std::pair<float, Vector3> maxPos{ -INFINITY,Vector3(0,0,0) };
			std::pair<float, Vector3> minPos{ INFINITY, Vector3(0, 0, 0) };
			float GetHeight() const { return maxPos.first - minPos.first; }
		};

		CylidnerEdge SearchCylinderEdge(const Vector3& origin, const Vector3& axis) const;
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
	static constexpr const char* EntityName = "FACE_SURFACE";
	static constexpr ESTEPEntityType ClassType = ESTEPFaceSurface;
	virtual ESTEPEntityType GetType() const { return ESTEPFaceSurface; }
	STEP_DEFINE_CAST(STEPFaceSurface)

	static void Fetch(STEPStruct& step, const STEPString& stepStr);

};

struct STEPAdvancedFace : public STEPFaceBase
{
	virtual ~STEPAdvancedFace() = default;
	static constexpr const char* EntityName = "ADVANCED_FACE";
	static constexpr ESTEPEntityType ClassType = ESTEPAdvancedFace;
	virtual ESTEPEntityType GetType() const { return ESTEPAdvancedFace; }
	STEP_DEFINE_CAST(STEPAdvancedFace)

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
	static constexpr const char* EntityName = "CLOSED_SHELL";
	static constexpr ESTEPEntityType ClassType = ESTEPClosedShell;
	virtual ESTEPEntityType GetType() const { return ESTEPClosedShell; }
	STEP_DEFINE_CAST(STEPClosedShell)

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	STEPShape CreateMesh(const STEPStruct& step);

};

struct STEPOpenShell : public STEPShell
{
	virtual ~STEPOpenShell() = default;
	static constexpr const char* EntityName = "OPEN_SHELL";
	static constexpr ESTEPEntityType ClassType = ESTEPOpenShell;
	virtual ESTEPEntityType GetType() const { return ESTEPOpenShell; }
	STEP_DEFINE_CAST(STEPOpenShell)

	static void Fetch(STEPStruct& step, const STEPString& stepStr);
	void FetchData(const STEPStruct& step);
	STEPShape CreateMesh(const STEPStruct& step);
};
}

#endif KI_STEP_ENTITY_H
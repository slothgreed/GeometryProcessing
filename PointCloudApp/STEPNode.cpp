#include "STEPNode.h"
#include "STEPTypes.h"
#include "FileUtility.h"
#include "RenderNode.h"
#include "SimpleShader.h"
#include "Polyline.h"
#include "KIMath.h"
#include "Utility.h"
#include "Primitives.h"
#include "DebugNode.h"
#include <functional>
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


struct STEPString
{
	STEPString():id(-1) {};
	~STEPString() {};
	int id;
	String value;
	String name;
	static STEPString Create(const String& str)
	{
		STEPString step;
		auto idToEntity = StringUtility::SplitAtFirst(str, '=');
		step.id = StringUtility::ToInt(StringUtility::Remove(idToEntity.first, '#'));
		auto nameToValue = StringUtility::SplitAtFirst(idToEntity.second, '(');
		step.name = StringUtility::TrimWhiteSpace(nameToValue.first);
		step.value = '(' + nameToValue.second;
		return step;
	}

	static Vector<String> SplitValue(const String& value)
	{	
		Vector<String> result;
		size_t start = 0;
		int depth = 0;  // 括弧の深さ
		for (size_t i = 0; i < value.size(); ++i) {
			char ch = value[i];

			// 括弧の開始
			if (ch == '(') {
				if (depth == 0) start = i + 1;  // 最初の '(' の後から開始
				++depth;
				// 括弧の終了
				} else if (ch == ')') {
					--depth;
					if (depth == 0) {
						// 最外の括弧が閉じたらリストとして追加
						result.push_back(value.substr(start, i - start));
					}
				}
				// 括弧内のカンマで区切り
				else if (depth == 1 && ch == ',' && (i == start || value[i - 1] != '\\')) {
					// カンマを区切りとして分ける（エスケープ文字も処理）
					result.push_back(value.substr(start, i - start));
					start = i + 1;
			}
		}
		return result;
	}

	String CreateRemoveLabelStr() const
	{
		auto valuePoint = StringUtility::After(value, ',');
		return StringUtility::RemoveLast(valuePoint, 2);
	}

	static String RemoveBracket(const String& str)
	{
		auto valuePoint = StringUtility::Remove(str, '(');
		valuePoint = StringUtility::Remove(valuePoint, ')');
		return valuePoint;
	}

	static bool ValueToRef(const String& str, int& value)
	{
		if (IsAsterisk(str)) { value = STEPEnum::ASTERISK; return true; }
		if (IsDollar(str)) { value = STEPEnum::DOLL; return true; }
		if (str.empty()) { return false; }

		// 先頭が '#' の場合は除去
		auto numStr = (str[0] == '#') ? str.substr(1) : str;

		// atoi() を使って整数変換

		value = StringUtility::ToInt(numStr.c_str());
		return true;
	}

	static bool ValueToFloat(const String& str, float& value)
	{
		value = StringUtility::ToFloat(str);
		return true;
	}

	static bool IsAsterisk(const String& str)
	{
		return StringUtility::Contains(str, "*");
	}

	// 文字がドル記号か判定
	static bool IsDollar(const String& str)
	{
		return StringUtility::Contains(str, "$");
	}

	static bool ValueToBool(const String& str, bool& value)
	{
		if (StringUtility::Contains(str, ".T.")) { value = true;  return true; }
		if (StringUtility::Contains(str, ".F.")) { value = false;  return true; }
		assert(0); return false;
	}

	String ToString() const
	{
		return
			"#" + StringUtility::ToString(id) +
			", " + name +
			", " + value;
	}
};

struct STEPEntityBase
{
	virtual ~STEPEntityBase() = default;
	int id = -1;
	String str;

	static void Fetch(STEPEntityBase* data, const STEPString& stepStr)
	{
		data->id = stepStr.id;
		data->str = stepStr.ToString();
	}
	virtual ESTEPEntityType GetType() const = 0;
	virtual void ShowUI(STEPUIContext& ui) = 0;
	bool ShowBranch(STEPUIContext& ui, bool select)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
		if (select) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		auto open = ImGui::TreeNodeEx(str.data(), flags);
		if (ImGui::IsItemClicked()) {
			ui.pSelect = this;
		}

		if (ImGui::BeginPopupContextItem()) {
			if (GetType() == ESTEPAdvancedFace) {
				if (ImGui::MenuItem("CreateNode")) {
					ui.pNode->AddDebugNode(ui, this);
				}
			}

			if (ImGui::MenuItem("DebugPrintf")) {
				DebugPrintf::StringStr("------------------------------\n");
				Printf(DebugOption());
				DebugPrintf::StringStr("------------------------------\n");
			}
			ImGui::EndPopup();
		}


		return open;
	}

	virtual void Printf(const DebugOption& option) = 0;

	void ShowLeaf(STEPUIContext& ui)
	{
		ImGui::TreeNodeEx(str.data(),
			ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	}

	void PrintfRaw()
	{
		DebugPrintf::StringStr(str); DebugPrintf::NewLine();
	}
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

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPPoint();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);
		if (!STEPString::ValueToFloat(values[0], data->data.pos.x)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[1], data->data.pos.y)) { assert(0); return; }
		if (values.size() == 3) {
			if (!STEPString::ValueToFloat(values[2], data->data.pos.z)) { assert(0); return; }
		}
		step.points[data->id] = data;
	}

	void FetchData(const STEPStruct& step){}

	void Printf(const DebugOption& option) { STEPEntityBase::PrintfRaw(); }
	void ShowUI(STEPUIContext& ui) { ShowLeaf(ui); }

};

struct STEPDirection : public STEPEntityBase
{
	virtual ~STEPDirection() = default;
	static constexpr const char* EntityName = "DIRECTION";
	static constexpr ESTEPEntityType ClassType = ESTEPDirection;
	virtual ESTEPEntityType GetType() const { return ESTEPDirection; }
	STEP_DEFINE_CAST(STEPDirection)

	Vector3 direction;
	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPDirection();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);
		if (!STEPString::ValueToFloat(values[0], data->direction.x)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[1], data->direction.y)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[2], data->direction.z)) { assert(0); return; }
		step.directions[data->id] = data;
	}

	void FetchData(const STEPStruct& step) {}
	void Printf(const DebugOption& option) { STEPEntityBase::PrintfRaw(); }
	void ShowUI(STEPUIContext& ui) { ShowLeaf(ui); }

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

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPVector();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.idRef)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[2], data->raw.length)) { assert(0); return; }

		step.vectors[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.driection = FindSetData2(step, step.directions, raw.idRef);
		data.vector = data.driection->direction;
		data.vector = glm::normalize(data.vector);
		data.vector *= raw.length;
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.driection) { data.driection->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.driection) { data.driection->ShowUI(ui); }
			ImGui::TreePop();
		}
	}
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
	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPLine();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.beginRef)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[2], data->raw.vectorRef)) { assert(0); return; }
		step.lines[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.point = FindSetData2(step, step.points, raw.beginRef);
		data.begin = data.point->data.pos;
		
		data.vector0 = FindSetData2(step, step.vectors, raw.vectorRef);
		data.vector = data.vector0->data.vector;
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.point) { data.point->Printf(option); }
		if (data.vector0) { data.vector0->Printf(option); }
	}


	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.point) { data.point->ShowUI(ui); }
			if (data.vector0) { data.vector0->ShowUI(ui); }
			ImGui::TreePop();
		}
	}

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

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPAxis2Placement3D();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.pointRef)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[2], data->raw.dirRef1)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[3], data->raw.dirRef2)) { assert(0); return; }

		step.axis2Placement3D[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.point0 = FindSetData2(step, step.points, raw.pointRef);
		data.point = data.point0->data.pos;

		data.direction1 = FindSetData2(step, step.directions, raw.dirRef1);
		if (data.direction1) data.dir1 = data.direction1->direction;


		data.direction2 = FindSetData2(step, step.directions, raw.dirRef2);
		if (data.direction2) data.dir2 = data.direction2->direction;
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.point0) { data.point0->Printf(option); }
		if (data.direction1) { data.direction1->Printf(option); }
		if (data.direction2) { data.direction2->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.point0) { data.point0->ShowUI(ui); }
			if (data.direction1) { data.direction1->ShowUI(ui); }
			if (data.direction2) { data.direction2->ShowUI(ui); }
			ImGui::TreePop();
		}
	}
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

		Polyline CreatePolyline() const
		{
			auto v = glm::cross(axis->data.dir1, axis->data.dir2);
			return Circle::CreateLine(rad, CIRCLE_SUBDIVISION_NUM, 
				glm::normalize(axis->data.dir2), glm::normalize(v), axis->data.point);
		}

		Polyline CreatePolyline(const Vector3& begin, const Vector3& end) const
		{
			auto v = glm::cross(axis->data.dir1, axis->data.dir2);
			return Circle::CreateArc(rad, CIRCLE_SUBDIVISION_NUM,
				glm::normalize(axis->data.dir2), glm::normalize(v), axis->data.point, begin, end);
		}
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPCircle();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.axisRef)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[2], data->raw.rad)) { assert(0); return; }
		step.circles[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.axis = FindSetData2(step, step.axis2Placement3D, raw.axisRef);
		data.rad = raw.rad;
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.axis) { data.axis->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.axis) { data.axis->ShowUI(ui); }
			ImGui::TreePop();
		}
	}
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

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPCylinderSurface();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.axisRef)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[2], data->raw.rad)) { assert(0); return; }
		step.cylinderSurface[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.axis = FindSetData2(step, step.axis2Placement3D, raw.axisRef);
		data.rad = raw.rad;
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.axis) { data.axis->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.axis) { data.axis->ShowUI(ui); }
			ImGui::TreePop();
		}
	}
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

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPPlane();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.idRef)) { assert(0); return; }
		step.planes[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.axis = FindSetData2(step, step.axis2Placement3D, raw.idRef);
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.axis) { data.axis->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.axis) { data.axis->ShowUI(ui); }
			ImGui::TreePop();
		}
	}

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
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPInterSectionCurve();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.curveId)) { assert(0); return; }
		auto split = STEPString::SplitValue(values[2]);
		if (!STEPString::ValueToRef(split[0], data->raw.geomId0)) { assert(0); return; }
		if (!STEPString::ValueToRef(split[1], data->raw.geomId1)) { assert(0); return; }
		if (StringUtility::Equal(values[3], ".CURVE_3D.")) { data->data.type = Data::Type::Curve3D; }
		step.interSectionCurve[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.curve0.pLine = FindSetData2(step, step.lines, raw.curveId);
		if (!data.curve0.IsActive()) {
			data.curve0.pCircle = FindSetData2(step, step.circles, raw.curveId);
		}
		data.surf0.pCylinderSurface = FindSetData2(step, step.cylinderSurface, raw.geomId0);
		if (!data.surf0.IsActive()) {
			data.surf0.pPlane = FindSetData2(step, step.planes, raw.geomId0);
		}

		data.surf1.pCylinderSurface = FindSetData2(step, step.cylinderSurface, raw.geomId1);
		if (!data.surf1.IsActive()) {
			data.surf1.pPlane = FindSetData2(step, step.planes, raw.geomId1);
		}

		if (!data.curve0.IsActive() ||
			!data.surf0.IsActive() || 
			!data.surf1.IsActive()) {
			assert(0);
		}
	}

	Polyline CreatePolyline(const Vector3& begin, const Vector3& end) const
	{
		return Polyline();
	}
	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.curve0.pLine) { data.curve0.pLine->Printf(option); }
		if (data.surf0.pPlane) { data.surf0.pPlane->Printf(option); }
		if (data.surf1.pCylinderSurface) { data.surf1.pCylinderSurface->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.curve0.pLine) { data.curve0.pLine->ShowUI(ui); }
			if (data.surf0.pPlane) { data.surf0.pPlane->ShowUI(ui); }
			if (data.surf0.pCylinderSurface) { data.surf0.pCylinderSurface->ShowUI(ui); }
			if (data.surf1.pPlane) { data.surf1.pPlane->ShowUI(ui); }
			if (data.surf1.pCylinderSurface) { data.surf1.pCylinderSurface->ShowUI(ui); }
			ImGui::TreePop();
		}
	}
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
	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPVertexPoint();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.idRef)) { assert(0); return; }
		step.vertexPoint[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.point = FindSetData2(step, step.points, raw.idRef);
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.point) { data.point->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.point) { data.point->ShowUI(ui); }
			ImGui::TreePop();
		}
	}
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

		Vector3 GetBegin() const
		{
			return sameSense ? begin : end;
		}

		Vector3 GetEnd() const
		{
			return sameSense ? end : begin;
		}
	};

	Polyline CreatePolyline() const
	{
		if (data.line) {
			return Polyline(Vector<Vector3>{ data.GetBegin(), data.GetEnd() });
		}

		if (data.circle) {
			if (MathHelper::IsSame(data.GetBegin(), data.GetEnd())) {
				return data.circle->data.CreatePolyline();
			} else {
				return data.circle->data.CreatePolyline(data.GetBegin(), data.GetEnd());
			}
		}

		if (data.intersectionCurve) {
			return data.intersectionCurve->data.CreatePolyline(data.GetBegin(), data.GetEnd());
		}

		return Polyline();
	}

	Data data;
	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPEdgeCurve();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.vertRef0)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[2], data->raw.vertRef1)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[3], data->raw.lineRef2)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[4], data->raw.sameSense)) { assert(0); return; }

		step.edgeCurve[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.pPoint0 = FindSetData2(step, step.vertexPoint, raw.vertRef0);
		data.begin = data.pPoint0->data.point->data.pos;

		data.pPoint1 = FindSetData2(step, step.vertexPoint, raw.vertRef1);
		data.end = data.pPoint1->data.point->data.pos;

		data.line = FindSetData2(step, step.lines, raw.lineRef2);
		if (!data.line) {
			data.circle = FindSetData2(step, step.circles, raw.lineRef2);
		}
		if (!data.line && !data.circle) {
			data.intersectionCurve = FindSetData2(step, step.interSectionCurve, raw.lineRef2);
		}

		data.sameSense = raw.sameSense;
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.pPoint0) { data.pPoint0->Printf(option); }
		if (data.pPoint1) { data.pPoint1->Printf(option); }
		if (data.line) { data.line->Printf(option); }
		if (data.circle) { data.circle->Printf(option); }
		if (data.intersectionCurve) { data.intersectionCurve->Printf(option); }
	}


	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.pPoint0) { data.pPoint0->ShowUI(ui); }
			if (data.pPoint1) { data.pPoint1->ShowUI(ui); }
			if (data.line) { data.line->ShowUI(ui); }
			if (data.circle) { data.circle->ShowUI(ui); }
			if (data.intersectionCurve) { data.intersectionCurve->ShowUI(ui); }
			ImGui::TreePop();
		}
	}
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

		bool IsLine() const
		{
			return edgeCurve->data.line != nullptr;
		}

		bool IsCircle() const
		{
			return edgeCurve->data.circle != nullptr;
		}

		Vector3 GetBegin() const
		{
			if (orient) {
				return begin;
			} else {
				return end;
			}
		}

		Vector3 GetEnd() const
		{
			if (orient) {
				return end;
			} else {
				return begin;
			}
		}

		float Length() const
		{
			return glm::length(GetBegin() - GetEnd());
		}

		Vector3 Dir() const
		{
			return GetEnd() - GetBegin();
		}

		Polyline CreatePolyline() const
		{
			auto polyline = edgeCurve->CreatePolyline();
			if (!orient) {
				polyline.Reverse();
			}
			return polyline;
		}
	};
	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPOrientedEdge();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->raw.vertRef0)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[2], data->raw.vertRef1)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[3], data->raw.edgeCurveRef2)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[4], data->raw.orient)) { assert(0); return; }
		step.orientedEdge[data->id] = data;
	}


	void FetchData(const STEPStruct& step)
	{
		data.edgeCurve = FindSetData2(step, step.edgeCurve, raw.edgeCurveRef2);
		if (raw.vertRef0 == STEPEnum::ASTERISK) {
			data.begin = data.edgeCurve->data.begin;
		} else {
			data.vertex0 = FindSetData2(step, step.vertexPoint, raw.vertRef0);
			data.begin = data.vertex0->data.point->data.pos;
		}

		if (raw.vertRef1 == STEPEnum::ASTERISK) {
			data.end = data.edgeCurve->data.end;
		} else {
			data.vertex1 = FindSetData2(step, step.vertexPoint, raw.vertRef1);
			data.end = data.vertex1->data.point->data.pos;
		}

		data.orient = raw.orient;
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.edgeCurve) { data.edgeCurve->Printf(option); }
		if (data.vertex0) { data.vertex0->Printf(option); }
		if (data.vertex1) { data.vertex1->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.edgeCurve) { data.edgeCurve->ShowUI(ui); }
			if (data.vertex0) { data.vertex0->ShowUI(ui); }
			if (data.vertex1) { data.vertex1->ShowUI(ui); }
			ImGui::TreePop();
		}
	}
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
		PolylineList CreatePolyline(int id) const
		{
			Vector<Vector3> lines(points.size() * 2);
			for (int i = 0; i < points.size(); i++) {
				lines[2 * i] = points[i]->data.pos;
				lines[2 * i + 1] = points[(i + 1)%points.size()]->data.pos;
			}
			return PolylineList(id, Polyline(std::move(lines), Polyline::DrawType::Lines));
		}
	};

	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPPolyLoop();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);

		data->raw.idRef.resize(values.size());
		for (int i = 0; i < data->raw.idRef.size(); i++) {
			if (!STEPString::ValueToRef(values[i], data->raw.idRef[i])) { assert(0); return; }
		}
		step.polyLoop[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.points.resize(raw.idRef.size());
		for (auto i = 0; i < raw.idRef.size(); i++) {
			data.points[i] = FindSetData2(step, step.points, raw.idRef[i]);
		}
	}
	
	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		for (size_t i = 0; i < data.points.size(); i++) {
			data.points[i]->Printf(option);
		}
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			for (size_t i = 0; i < data.points.size(); i++) {
				data.points[i]->ShowUI(ui);
			}

			ImGui::TreePop();
		}
	}
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
		PolylineList CreatePolyline() const
		{
			Vector<Vector3> points;
			PolylineList polyline;
			for (size_t i = 0; i < orientedEdges.size(); ++i) {
				if (orientedEdges[i] == nullptr) { continue; }
				if (orientedEdges[i]->data.IsCircle()) {
					polyline.Add(orientedEdges[i]->id,orientedEdges[i]->data.CreatePolyline());
				} else {
					Polyline line;
					line.Add(orientedEdges[i]->data.GetBegin());
					line.Add(orientedEdges[i]->data.GetEnd());
					polyline.Add(orientedEdges[i]->id, std::move(line));
				}
			}

			return polyline;
		}
	};

	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPEdgeLoop();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);

		data->raw.idRef.resize(values.size());
		for (int i = 0; i < data->raw.idRef.size(); i++) {
			if (!STEPString::ValueToRef(values[i], data->raw.idRef[i])) { assert(0); return; }
		}
		step.edgeLoop[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		data.orientedEdges.resize(raw.idRef.size());
		for (auto i = 0; i < raw.idRef.size(); i++) {
			data.orientedEdges[i] = FindSetData2(step, step.orientedEdge, raw.idRef[i]);
		}
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		for (auto orientedEdge : data.orientedEdges) {
			if (orientedEdge == nullptr) { continue; }
			orientedEdge->Printf(option);
		}
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			for (auto orientedEdge : data.orientedEdges) {
				if (orientedEdge == nullptr) { continue; }
				orientedEdge->ShowUI(ui);
			}
			ImGui::TreePop();
		}
	}
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

		PolylineList CreatePolyline() const
		{
			if (edgeLoop) { return edgeLoop->data.CreatePolyline(); }
			if (polyLoop) { return polyLoop->data.CreatePolyline(polyLoop->id); }

			return PolylineList();
		}
	};

	Data data;

	static void Fetch(STEPStruct& step, const STEPString& stepStr, STEPFaceBoundBase* data)
	{
		auto values = STEPString::SplitValue(stepStr.value);
		STEPEntityBase::Fetch(data, stepStr);
		if (!STEPString::ValueToRef(values[1], data->raw.idRef0)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[2], data->raw.orient)) { assert(0); return; }
	}

	void FetchData(const STEPStruct& step)
	{
		data.edgeLoop = FindSetData2(step, step.edgeLoop, raw.idRef0);
		if (!data.edgeLoop) {
			data.polyLoop = FindSetData2(step, step.polyLoop, raw.idRef0);
		}

		data.orient = raw.orient;
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();
		if (data.edgeLoop) { data.edgeLoop->Printf(option); }
		if (data.polyLoop) { data.polyLoop->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			if (data.edgeLoop) { data.edgeLoop->ShowUI(ui); }
			if (data.polyLoop) { data.polyLoop->ShowUI(ui); }
			ImGui::TreePop();
		}
	}
};

struct STEPFaceOuterBound : public STEPFaceBoundBase
{
	virtual ~STEPFaceOuterBound() = default;
	static constexpr const char* EntityName = "FACE_OUTER_BOUND";
	static constexpr ESTEPEntityType ClassType = ESTEPFaceOuterBound;
	virtual ESTEPEntityType GetType() const { return ESTEPFaceOuterBound; }
	STEP_DEFINE_CAST(STEPFaceOuterBound)

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPFaceOuterBound();
		STEPFaceBoundBase::Fetch(step, stepStr, data);
		step.faceOuterBound[data->id] = data;
	}
};

struct STEPFaceBound : public STEPFaceBoundBase
{
	virtual ~STEPFaceBound() = default;
	static constexpr const char* EntityName = "FACE_BOUND";
	static constexpr ESTEPEntityType ClassType = ESTEPFaceBound;
	virtual ESTEPEntityType GetType() const { return ESTEPFaceBound; }
	STEP_DEFINE_CAST(STEPFaceBound)

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPFaceBound();
		STEPFaceBoundBase::Fetch(step, stepStr, data);
		step.faceBound[data->id] = data;
	}
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

		CylidnerEdge SearchCylinderEdge(const Vector3& origin, const Vector3& axis) const
		{
			CylidnerEdge ret;
			for (const auto& face : faceBound) {
				auto edgeLoop = face->data.edgeLoop;
				if (!edgeLoop) { continue; }
				for (const auto& edge : edgeLoop->data.orientedEdges) {
					auto v0 = glm::dot(edge->data.GetBegin() - origin, axis);
					auto v1 = glm::dot(edge->data.GetEnd() - origin, axis);
					if (ret.maxPos.first < v0) { ret.maxPos.first = v0; ret.maxPos.second = edge->data.GetBegin();}
					if (ret.maxPos.first < v1) { ret.maxPos.first = v1; ret.maxPos.second = edge->data.GetEnd(); }
					
					if (ret.minPos.first > v0) { ret.minPos.first = v0; ret.minPos.second = edge->data.GetBegin(); }
					if (ret.minPos.first > v1) { ret.minPos.first = v1; ret.minPos.second = edge->data.GetEnd(); }

					if (edge->data.IsCircle()) { ret.circle = &edge->data; }
				}
			}

			for (const auto& face : faceOuterBound) {
				auto edgeLoop = face->data.edgeLoop;
				if (!edgeLoop) { continue; }
				for (const auto& edge : edgeLoop->data.orientedEdges) {
					auto v0 = glm::dot(edge->data.GetBegin() - origin, axis);
					auto v1 = glm::dot(edge->data.GetEnd() - origin, axis);
					if (ret.maxPos.first < v0) { ret.maxPos.first = v0; ret.maxPos.second = edge->data.GetBegin(); }
					if (ret.maxPos.first < v1) { ret.maxPos.first = v1; ret.maxPos.second = edge->data.GetEnd(); }

					if (ret.minPos.first > v0) { ret.minPos.first = v0; ret.minPos.second = edge->data.GetBegin(); }
					if (ret.minPos.first > v1) { ret.minPos.first = v1; ret.minPos.second = edge->data.GetEnd(); }
					if (edge->data.IsCircle()) { ret.circle = &edge->data; }
				}
			}

			return ret;
		}
		
		PolylineList CreateBoundPolyline() const
		{
			PolylineList bound;
			for (const auto& face : faceBound) { bound.Add(face->data.CreatePolyline()); }
			return bound;
		}

		PolylineList CreateOuterBoundPolyline() const
		{
			PolylineList outerBound;
			for (const auto& face : faceOuterBound) { outerBound.Add(face->data.CreatePolyline()); }
			return outerBound;
		}

		Mesh CreateMesh(const Polyline& bound, const Polyline& outerBound) const
		{
			if (plane) {
				if (bound.PointNum() == 0 && outerBound.PointNum() == 0) { return Mesh(); }
				auto normal = glm::normalize(plane->data.axis->data.Normal());
				if (!orient) {
					normal = -normal;
				}
				if (bound.PointNum() && outerBound.PointNum()) {
					return Polyline::CreateMesh(outerBound, bound, normal);
				} else if (bound.PointNum()) {
					return Polyline::CreateMesh(bound, Polyline(), normal);
				} else if (outerBound.PointNum()) {
					return Polyline::CreateMesh(outerBound, Polyline(), normal);
				}
			} else if (cylinder) {

				auto normal = cylinder->data.axis->data.Normal();
				//if (!orient) { normal = -normal; }

				auto edge = SearchCylinderEdge(
					cylinder->data.axis->data.point, normal);
				if (!edge.IsActive()) { return Mesh(); }
				Vector3 origin = cylinder->data.axis->data.point +
					normal * edge.minPos.first;

				auto begin = edge.circle->GetBegin();
				auto end = edge.circle->GetEnd();
				//if (!orient) { begin = -begin; end = -end; }
				return Cylinder::CreateSideMesh(
					origin,
					normal,
					edge.circle->GetBegin(),
					edge.circle->GetEnd(),
					cylinder->data.rad,
					edge.GetHeight(),
					orient,
					CIRCLE_SUBDIVISION_NUM,
					CIRCLE_SUBDIVISION_NUM);
			}

			return Mesh();
		}
	};

	void FetchData(const STEPStruct& step)
	{
		for (auto i = 0; i < raw.faceRef0.size(); i++) {
			auto pFaceOuterBound = FindSetData2(step, step.faceOuterBound, raw.faceRef0[i]);
			if (pFaceOuterBound) { data.faceOuterBound.push_back(pFaceOuterBound); continue; }
			auto pFaceBound = FindSetData2(step, step.faceBound, raw.faceRef0[i]);
			if (pFaceBound) { data.faceBound.push_back(pFaceBound); continue; }
		}
		data.plane = FindSetData2(step, step.planes, raw.geomRef1);
		data.cylinder = FindSetData2(step, step.cylinderSurface, raw.geomRef1);
		data.orient = raw.orient;
	}

	Raw raw;
	Data data;

	void Printf(const DebugOption& option)
	{
		PrintfRaw();

		for (auto& outerBound : data.faceOuterBound) {	outerBound->Printf(option);	}
		for (auto& faceBound : data.faceBound) { faceBound->Printf(option); }
		if (data.plane) { data.plane->Printf(option); }
		if (data.cylinder) { data.cylinder->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			for (int i = 0; i < data.faceOuterBound.size(); i++) {
				data.faceOuterBound[i]->ShowUI(ui);
			}
			for (int i = 0; i < data.faceBound.size(); i++) {
				data.faceBound[i]->ShowUI(ui);
			}

			if (data.plane) { data.plane->ShowUI(ui); }
			if (data.cylinder) { data.cylinder->ShowUI(ui); }

			ImGui::TreePop();
		}
	}
};

struct STEPFaceSurface : public STEPFaceBase
{
	virtual ~STEPFaceSurface() = default;
	static constexpr const char* EntityName = "FACE_SURFACE";
	static constexpr ESTEPEntityType ClassType = ESTEPFaceSurface;
	virtual ESTEPEntityType GetType() const { return ESTEPFaceSurface; }
	STEP_DEFINE_CAST(STEPFaceSurface)

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto pData = new STEPFaceSurface();
		STEPEntityBase::Fetch(pData, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		auto faces = STEPString::SplitValue(values[1]);
		pData->raw.faceRef0.resize(faces.size());
		for (int i = 0; i < faces.size(); i++) {
			if (!STEPString::ValueToRef(faces[i], pData->raw.faceRef0[i])) { assert(0); return; }
		}

		if (!STEPString::ValueToRef(values[2], pData->raw.geomRef1)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[3], pData->raw.orient)) { assert(0); return; }
		step.faceSurface[pData->id] = pData;
	}

};

struct STEPAdvancedFace : public STEPFaceBase
{
	virtual ~STEPAdvancedFace() = default;
	static constexpr const char* EntityName = "ADVANCED_FACE";
	static constexpr ESTEPEntityType ClassType = ESTEPAdvancedFace;
	virtual ESTEPEntityType GetType() const { return ESTEPAdvancedFace; }
	STEP_DEFINE_CAST(STEPAdvancedFace)

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPAdvancedFace();
		STEPEntityBase::Fetch(data, stepStr);
		auto values = STEPString::SplitValue(stepStr.value);
		auto faces = STEPString::SplitValue(values[1]);
		data->raw.faceRef0.resize(faces.size());
		for (int i = 0; i < faces.size(); i++) {
			if (!STEPString::ValueToRef(faces[i], data->raw.faceRef0[i])) { assert(0); return; }
		}

		if (!STEPString::ValueToRef(values[2], data->raw.geomRef1)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[3], data->raw.orient)) { assert(0); return; }
		step.advancedFace[data->id] = data;
	}

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

	static void Fetch(STEPStruct& step, const STEPString& stepStr, STEPShell* pShell)
	{
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);
		pShell->raw.faceRef.resize(values.size());
		for (int i = 0; i < values.size(); i++) {
			if (!STEPString::ValueToRef(values[i], pShell->raw.faceRef[i])) { assert(0); return; }
		}
		STEPEntityBase::Fetch(pShell, stepStr);
	}

	void FetchData(const STEPStruct& step, STEPShell::Data* data)
	{
		data->Clear();
		for (auto i = 0; i < raw.faceRef.size(); i++) {
			auto pAdvanedFace = FindSetData2(step, step.advancedFace, raw.faceRef[i]);
			if (pAdvanedFace) { data->advancedFace.push_back(pAdvanedFace); continue; }

			auto pFaceSurface = FindSetData2(step, step.faceSurface, raw.faceRef[i]);
			if (pFaceSurface) { data->faceSurface.push_back(pFaceSurface); }
		}

	}

	void CreateMesh(const STEPShell& step, STEPShape& shape)
	{
		for (const auto& advancedFace : step.data.advancedFace) {
			if (advancedFace) {
				auto bound = advancedFace->data.CreateBoundPolyline();
				auto outerBound = advancedFace->data.CreateOuterBoundPolyline();
				shape.AddMesh(advancedFace->id,
					std::move(advancedFace->data.CreateMesh(bound.CreateMerge(), outerBound.CreateMerge()).ConvertTriangles()));
				shape.AddPolyline(std::move(bound));
				shape.AddPolyline(std::move(outerBound));
			}
		}
		for (const auto& faceSurface : step.data.faceSurface) {
			if (faceSurface) {
				auto bound = faceSurface->data.CreateBoundPolyline();
				auto outerBound = faceSurface->data.CreateOuterBoundPolyline();
				shape.AddMesh(faceSurface->id, std::move(faceSurface->data.CreateMesh(bound.CreateMerge(), outerBound.CreateMerge()).ConvertTriangles()));
				shape.AddPolyline(std::move(bound));
				shape.AddPolyline(std::move(outerBound));
			}
		}
	}

	void Printf(const DebugOption& option)
	{
		PrintfRaw();

		for (auto& face : data.advancedFace) { face->Printf(option); }
		for (auto& surface : data.faceSurface) { surface->Printf(option); }
	}

	void ShowUI(STEPUIContext& ui)
	{
		if (ShowBranch(ui, ui.IsSelect(id))) {
			for (int i = 0; i < data.advancedFace.size(); i++) {
				data.advancedFace[i]->ShowUI(ui);
			}
			for (int i = 0; i < data.faceSurface.size(); i++) {
				data.faceSurface[i]->ShowUI(ui);
			}

			ImGui::TreePop();
		}
	}

};
struct STEPClosedShell : public STEPShell
{
	virtual ~STEPClosedShell() = default;
	static constexpr const char* EntityName = "CLOSED_SHELL";
	static constexpr ESTEPEntityType ClassType = ESTEPClosedShell;
	virtual ESTEPEntityType GetType() const { return ESTEPClosedShell; }
	STEP_DEFINE_CAST(STEPClosedShell)

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPClosedShell();
		STEPShell::Fetch(step, stepStr, data);
		step.closedShell[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		STEPShell::FetchData(step, &data);
	}

	STEPShape CreateMesh(const STEPStruct& step)
	{
		FetchData(step);
		STEPShape mesh;
		STEPShell::CreateMesh(*this, mesh);
		return mesh;
	}

};

struct STEPOpenShell : public STEPShell
{
	virtual ~STEPOpenShell() = default;
	static constexpr const char* EntityName = "OPEN_SHELL";
	static constexpr ESTEPEntityType ClassType = ESTEPOpenShell;
	virtual ESTEPEntityType GetType() const { return ESTEPOpenShell; }
	STEP_DEFINE_CAST(STEPOpenShell)


	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPOpenShell();
		STEPShell::Fetch(step, stepStr, data);
		step.openShell[data->id] = data;
	}

	void FetchData(const STEPStruct& step)
	{
		STEPShell::FetchData(step, &data);
	}

	STEPShape CreateMesh(const STEPStruct& step)
	{
		FetchData(step);
		STEPShape shape;
		STEPShell::CreateMesh(*this, shape);
		return shape;
	}
};

void NotDefineEntity(const String& str)
{
	// 形状に不必要なEntity
	// マテリアル情報も含まれる。
	static const std::unordered_set<std::string> handlers = {
		"APPLICATION_CONTEXT",
		"APPLICATION_PROTOCOL_DEFINITION",
		"APPROVAL",
		"APPROVAL_DATE_TIME",
		"APPROVAL_PERSON_ORGANIZATION",
		"APPROVAL_ROLE",
		"APPROVAL_STATUS",
		"CALENDAR_DATE",
		"CC_DESIGN_APPROVAL",
		"CC_DESIGN_DATE_AND_TIME_ASSIGNMENT",
		"CC_DESIGN_PERSON_AND_ORGANIZATION_ASSIGNMENT",
		"CC_DESIGN_SECURITY_CLASSIFICATION",
		"COORDINATED_UNIVERSAL_TIME_OFFSET",
		"DATA_TIME_ROLE",
		"DATE_AND_TIME",
		"DATE_TIME_ROLE",
		"DESIGN_CONTEXT",
		"DIMENSIONAL_EXPONENTS",
		"DRAUGHTING_PRE_DEFINED_COLOUR",
		"FILL_AREA_STYLE",
		"FILL_AREA_STYLE_COLOUR", // マテリアル
		"LOCAL_TIME",
		"MECHANICAL_CONTEXT",
		"ORGANIZATION",
		"PERSON",
		"PERSON_AND_ORGANIZATION",
		"PERSON_AND_ORGANIZATION_ROLE",
		"PLANE_ANGLE_MEASURE_WITH_UNIT", // 角度orラジアン
		"PRESENTATION_STYLE_ASSIGNMENT",
		"PRODUCT",
		"PRODUCT_CATEGORY_RELATIONSHIP",
		"PRODUCT_CONTEXT",
		"PRODUCT_DEFINITION",
		"PRODUCT_DEFINITION_CONTEXT",
		"PRODUCT_DEFINITION_FORMATION_WITH_SPECIFIED_SOURCE",
		"PRODUCT_DEFINITION_SHAPE",
		"PRODUCT_RELATED_PRODUCT_CATEGORY",
		"SECURITY_CLASSIFICATION",
		"SECURITY_CLASSIFICATION_LEVEL",

		"STYLED_ITEM",
		"SURFACE_SIDE_STYLE",    // 裏面表面両面
		"SURFACE_STYLE_FILL_AREA", // 塗りつぶし
		"SURFACE_STYLE_USAGE",
	};

	"LENGTH_MEASURE_WITH_UNIT";
	"ADVANCED_BREP_SHAPE_REPRESENTATION";
	"MANIFOLD_SOLID_BREP";
	"UNCERTAINTY_MEASURE_WITH_UNIT";
	"SHAPE_DEFINITION_REPRESENTATION";
	"SHAPE_REPRESENTATION_RELATIONSHIP";

	auto stepStr = STEPString::Create(str);
	for (const auto& key : handlers) {
		if (StringUtility::Equal(stepStr.name, key)) {
			return;
		}
	}

	// 検討していないエンティティ
	printf("%s\n", str.data());
}


RenderNode* STEPLoader::CreateRenderNode(const String& name, const Shared<STEPStruct>& step)
{
	BDB bdb;
	Vector<STEPShape> shapes;
	for (const auto& shell : step->closedShell) {
		auto shape = shell.second->CreateMesh(*step);
		bdb.Add(shape.CreateBDB());
		shapes.push_back(std::move(shape));
	}

	for (const auto& shell : step->openShell) {
		auto shape = shell.second->CreateMesh(*step);
		bdb.Add(shape.CreateBDB());
		shapes.push_back(std::move(shape));
	}

	STEPRenderNode* pRenderNode = new STEPRenderNode(name, step);
	pRenderNode->SetBoundBox(bdb);
	pRenderNode->SetShape(std::move(shapes));

	return pRenderNode;
}

RenderNode* STEPLoader::Load(const String& name, int index, bool saveOriginal)
{
	auto extension = FileUtility::GetExtension(name);
	if (!(extension == ".step" || extension == ".stp")) { return nullptr; }
	Vector<String> contents;
	if (!FileUtility::Load(name, contents)) { return nullptr; }
	FileWriter writer;
	if (saveOriginal) {
		writer.Open(name + ".orig");
	}

	int dataIndex = -1;
	for (int i = 0; i < contents.size(); i++) {
		if (contents[i] == "DATA;") {
			dataIndex = i;
			break;
		}
	}

	if (dataIndex == -1) { return nullptr; }

	auto pStep = std::make_shared<STEPStruct>();
	auto& step = *pStep;
	for (int i = dataIndex + 1; i < contents.size(); i++) {
		if (contents[i] == "ENDSEC;")break;
		auto content = contents[i];
		if (content[content.size() - 1] != ';') {
			int add = 0;
			for (int j = i + 1; j < contents.size(); j++) {
				content += contents[j];
				add++;
				if (contents[j][contents[j].size() - 1] == ';') {
					break;
				}
			}
			i += add;
		}
		bool writeEntity = true;
		auto stepStr = STEPString::Create(content);
		if (StringUtility::Equal(stepStr.name, STEPPoint::EntityName)) { STEPPoint::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPDirection::EntityName)) { STEPDirection::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPVector::EntityName)) { STEPVector::Fetch(step, stepStr);}
		else if (StringUtility::Equal(stepStr.name, STEPPlane::EntityName)) { STEPPlane::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPLine::EntityName)) { STEPLine::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPAxis2Placement3D::EntityName)) { STEPAxis2Placement3D::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPEdgeCurve::EntityName)) { STEPEdgeCurve::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPVertexPoint::EntityName)) { STEPVertexPoint::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPEdgeLoop::EntityName)) { STEPEdgeLoop::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPPolyLoop::EntityName)) { STEPPolyLoop::Fetch(step, stepStr); } else if (StringUtility::Contains(stepStr.name, STEPFaceBound::EntityName)) { STEPFaceBound::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPFaceOuterBound::EntityName)) { STEPFaceOuterBound::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPOrientedEdge::EntityName)) { STEPOrientedEdge::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPAdvancedFace::EntityName)) { STEPAdvancedFace::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPFaceSurface::EntityName)) { STEPFaceSurface::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPClosedShell::EntityName)) { STEPClosedShell::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPOpenShell::EntityName)) { STEPOpenShell::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPCircle::EntityName)) { STEPCircle::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPCylinderSurface::EntityName)) { STEPCylinderSurface::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPInterSectionCurve::EntityName)) { STEPInterSectionCurve::Fetch(step, stepStr); }
		else { NotDefineEntity(content); writeEntity = false; }

		if (writeEntity && saveOriginal) {
			writer.Write(content, true);
		}
	}

	if (saveOriginal) {
		writer.Close();
	}

	return CreateRenderNode(name + IntToString(index), pStep);
}

void STEPRenderNode::RenderBatch::Allocate(GLuint type)
{
	if (pointNum > 0) {
		pPosition = std::make_unique<GLBuffer>();
		pPosition->Create(DATA_FLOAT, pointNum, sizeof(Vector3), nullptr);
	}

	if (indexNum > 0) {
		pIndex = std::make_unique<GLBuffer>();
		pIndex->Create(DATA_UINT, indexNum, sizeof(UInt), nullptr);
	}
	drawType = type;
}

void STEPRenderNode::BuildGLResource()
{
	if (m_shape.size() == 0) { return; }
	if (m_gpu.IsActive()) { return; }

	auto cube = Cube::CreateLine(GetBoundBox().Min(), GetBoundBox().Max());
	m_gpu.bdb.drawType = GL_LINES;
	m_gpu.bdb.pPosition = std::make_unique<GLBuffer>();
	m_gpu.bdb.pIndex = std::make_unique<GLBuffer>();

	m_gpu.bdb.pPosition->Create(cube.Position());
	m_gpu.bdb.pIndex->Create(cube.Index());

	for (const auto& shape : m_shape) {
		for (const auto& m : shape.meshs) {
			const auto& mesh = m.second;
			if (mesh.GetIndexs().size() != 0) {
				m_gpu.triangleIndex.pointNum += mesh.GetPoints().size();
				m_gpu.triangleIndex.indexNum += mesh.GetIndexs().size();
			} else {
				m_gpu.triangle.pointNum += mesh.GetPoints().size();
			}
		}

		for (const auto& pList : shape.polylineList) {
			for (const auto& p : pList.GetPolylines()) {
				const auto& polyline = p.second;
				if (polyline.GetPoints().size() == 0) { continue; }
				if (polyline.GetDrawType() == GL_LINES) {
					if (polyline.GetIndexs().size() != 0) {
						m_gpu.lineIndex.pointNum += polyline.GetPoints().size();
						m_gpu.lineIndex.indexNum += polyline.GetIndexs().size();
					} else {
						m_gpu.line.pointNum += polyline.GetPoints().size();
					}
				} else 	if (polyline.GetDrawType() == GL_LINE_STRIP) {
					if (polyline.GetIndexs().size() != 0) {
						m_gpu.lineStripIndex.pointNum += polyline.GetPoints().size();
						m_gpu.lineStripIndex.indexNum += polyline.GetIndexs().size();
					} else {
						m_gpu.lineStrip.pointNum += polyline.GetPoints().size();
					}
				} else 	if (polyline.GetDrawType() == GL_LINE_LOOP) {
					if (polyline.GetIndexs().size() != 0) {
						m_gpu.lineLoopIndex.pointNum += polyline.GetPoints().size();
						m_gpu.lineLoopIndex.indexNum += polyline.GetIndexs().size();
					} else {
						m_gpu.lineLoop.pointNum += polyline.GetPoints().size();
					}
				}
			}
		}
	}

	m_gpu.triangle.Allocate(GL_TRIANGLES);	m_gpu.triangleIndex.Allocate(GL_TRIANGLES);
	m_gpu.line.Allocate(GL_LINES);	m_gpu.lineIndex.Allocate(GL_LINES);
	m_gpu.lineStrip.Allocate(GL_LINE_STRIP); m_gpu.lineStripIndex.Allocate(GL_LINE_STRIP);
	m_gpu.lineLoop.Allocate(GL_LINE_LOOP); m_gpu.lineLoop.Allocate(GL_LINE_LOOP);

	size_t trianglePointOffset = 0;
	size_t triangleIndexPointOffset = 0;
	size_t triangleIndexOffset = 0;
	size_t linePointOffset = 0;
	size_t lineIndexPointOffset = 0;
	size_t lineIndexOffset = 0;
	size_t lineStripPointOffset = 0;
	size_t lineStripIndexPointOffset = 0;
	size_t lineStripIndexOffset = 0;
	size_t lineLoopPointOffset = 0;
	size_t lineLoopIndexPointOffset = 0;
	size_t lineLoopIndexOffset = 0;
	for (const auto& shape : m_shape) {
		for (const auto& m : shape.meshs) {
			const auto& mesh = m.second;
			if (mesh.GetPoints().size() == 0) { continue; }
			if (mesh.GetIndexs().size()) {
				m_gpu.triangleIndex.pPosition->BufferSubData(triangleIndexPointOffset, mesh.GetPoints());
				triangleIndexPointOffset += mesh.GetPoints().size();

				m_gpu.triangleIndex.AddEntity(m.first, triangleIndexOffset, mesh.GetIndexs().size());
				m_gpu.triangleIndex.pIndex->BufferSubData(triangleIndexOffset, mesh.GetIndexs());
				triangleIndexOffset += mesh.GetIndexs().size();
			} else {
				m_gpu.triangle.AddEntity(m.first, trianglePointOffset, mesh.GetPoints().size());
				m_gpu.triangle.pPosition->BufferSubData(trianglePointOffset, mesh.GetPoints());
				trianglePointOffset += mesh.GetPoints().size();
			}
		}

		for (const auto& pList : shape.polylineList) {
			for (const auto& p : pList.GetPolylines()) {
				const auto& polyline = p.second;
				if (polyline.GetPoints().size() == 0) { continue; }
				if (polyline.GetDrawType() == GL_LINES) {
					if (polyline.GetIndexs().size()) {
						m_gpu.lineIndex.pPosition->BufferSubData(lineIndexPointOffset, polyline.GetPoints());
						lineIndexPointOffset += polyline.GetPoints().size();

						m_gpu.lineIndex.AddEntity(p.first, lineIndexOffset, polyline.GetIndexs().size());
						m_gpu.lineIndex.pIndex->BufferSubData(lineIndexOffset, polyline.GetIndexs());
						lineIndexOffset += polyline.GetIndexs().size();
					} else {
						m_gpu.line.AddEntity(p.first, linePointOffset, polyline.GetPoints().size());
						m_gpu.line.pPosition->BufferSubData(linePointOffset, polyline.GetPoints());
						linePointOffset += polyline.GetPoints().size();
					}
				} else if (polyline.GetDrawType() == GL_LINE_STRIP) {
					if (polyline.GetIndexs().size()) {
						m_gpu.lineStripIndex.pPosition->BufferSubData(lineStripIndexPointOffset, polyline.GetPoints());
						lineStripIndexPointOffset += polyline.GetPoints().size();

						m_gpu.lineStripIndex.AddEntity(p.first, lineStripIndexOffset, polyline.GetIndexs().size());
						m_gpu.lineStripIndex.pIndex->BufferSubData(lineStripIndexOffset, polyline.GetIndexs());
						lineStripIndexOffset += polyline.GetIndexs().size();
					} else {
						m_gpu.lineStrip.AddEntity(p.first, lineStripPointOffset, polyline.GetPoints().size());
						m_gpu.lineStrip.pPosition->BufferSubData(lineStripPointOffset, polyline.GetPoints());
						lineStripPointOffset += polyline.GetPoints().size();
					}
				} else if (polyline.GetDrawType() == GL_LINE_LOOP) {
					if (polyline.GetIndexs().size()) {
						m_gpu.lineLoopIndex.pPosition->BufferSubData(lineLoopIndexPointOffset, polyline.GetPoints());
						lineLoopIndexPointOffset += polyline.GetPoints().size();

						m_gpu.lineLoopIndex.AddEntity(p.first, lineLoopIndexOffset, polyline.GetIndexs().size());
						m_gpu.lineLoopIndex.pIndex->BufferSubData(lineLoopIndexOffset, polyline.GetIndexs());
						lineLoopIndexOffset += polyline.GetIndexs().size();
					} else {
						m_gpu.lineLoop.AddEntity(p.first, lineLoopPointOffset, polyline.GetPoints().size());
						m_gpu.lineLoop.pPosition->BufferSubData(lineLoopPointOffset, polyline.GetPoints());
						lineLoopPointOffset += polyline.GetPoints().size();
					}
				}
			}
		}
	}
}
void STEPRenderNode::AddDebugNode(STEPUIContext& context, const STEPEntityBase* pBase)
{
	auto pFace = STEPAdvancedFace::Cast(pBase);
	auto bound = pFace->data.CreateBoundPolyline().CreateMerge();
	auto outerBound = pFace->data.CreateOuterBoundPolyline().CreateMerge();
	auto mesh = pFace->data.CreateMesh(bound, outerBound);
	context.ui->ClearDebugNode();
	if (bound.LineNum() != 0) {
		context.ui->AddDebugNode(std::make_shared<PolylineNode>(pBase->str + "::Bound", bound));
	}
	if (outerBound.LineNum() != 0) {
		context.ui->AddDebugNode(std::make_shared<PolylineNode>(pBase->str + "::OuterBound", outerBound));
	}
	if (mesh.TriangleNum() != 0) {
		context.ui->AddDebugNode(std::make_shared<MeshNode>(pBase->str + "::Mesh", mesh));
	}
}
void STEPRenderNode::DrawNode(const DrawContext& context)
{
	BuildGLResource();
	if (!m_gpu.IsActive()) { return; }
	Vector3 selectColor = Vector3(1.0f, 1.0f, 1.0f);
	auto pResource = context.pResource;
	auto matrix = GetTranslateMatrix() * GetScaleMatrix() * m_rotateMatrix;
	auto pSimpleShader = pResource->GetShaderTable()->GetSimpleShader();
	pSimpleShader->Use();
	pSimpleShader->SetModel(matrix);
	pSimpleShader->SetCamera(context.pResource->GetCameraBuffer());
	if (m_ui.visibleBDB) {
		pSimpleShader->SetColor(Vector3(0, 0, 1));
		pSimpleShader->SetPosition(m_gpu.bdb.pPosition.get());
		pSimpleShader->DrawElement(m_gpu.bdb.drawType, m_gpu.bdb.pIndex.get());
	}

	pSimpleShader->SetCamera(pResource->GetCameraBuffer());
	pSimpleShader->SetModel(matrix);
	if (m_gpu.line.pPosition) {
		pSimpleShader->SetPosition(m_gpu.line.pPosition.get());
		pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
		pSimpleShader->DrawArray(m_gpu.line.drawType, m_gpu.line.pPosition.get());
		auto entity = m_gpu.line.FindEntity(uiContext.GetSelectId());
		if (entity.IsActive()) {
			pSimpleShader->SetColor(selectColor);
			pSimpleShader->DrawArray(m_gpu.line.drawType, entity.first, entity.num);
		}
	}

	if (m_gpu.lineStrip.pPosition) {
		pSimpleShader->SetPosition(m_gpu.lineStrip.pPosition.get());
		pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
		pSimpleShader->DrawArray(m_gpu.lineStrip.drawType, m_gpu.lineStrip.pPosition.get());
		auto entity = m_gpu.lineStrip.FindEntity(uiContext.GetSelectId());
		if (entity.IsActive()) {
			pSimpleShader->SetColor(selectColor);
			pSimpleShader->DrawArray(m_gpu.lineStrip.drawType, entity.first, entity.num);
		}
	}

	if (m_gpu.lineLoop.pPosition) {
		pSimpleShader->SetPosition(m_gpu.lineLoop.pPosition.get());
		pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
		pSimpleShader->DrawArray(m_gpu.lineLoop.drawType, m_gpu.lineLoop.pPosition.get());
		auto entity = m_gpu.lineLoop.FindEntity(uiContext.GetSelectId());
		if (entity.IsActive()) {
			pSimpleShader->SetColor(selectColor);
			pSimpleShader->DrawArray(m_gpu.lineLoop.drawType, entity.first, entity.num);
		}
	}

	if (m_gpu.lineIndex.pPosition) {
		pSimpleShader->SetPosition(m_gpu.lineIndex.pPosition.get());
		pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
		pSimpleShader->DrawElement(m_gpu.lineIndex.drawType, m_gpu.lineIndex.pIndex.get());
		auto entity = m_gpu.lineIndex.FindEntity(uiContext.GetSelectId());
		if (entity.IsActive()) {
			pSimpleShader->SetColor(selectColor);
			pSimpleShader->DrawArray(m_gpu.lineIndex.drawType, entity.first, entity.num);
		}
	}

	if (m_gpu.lineStripIndex.pPosition) {
		pSimpleShader->SetPosition(m_gpu.lineStripIndex.pPosition.get());
		pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
		pSimpleShader->DrawElement(m_gpu.lineStripIndex.drawType, m_gpu.lineStripIndex.pIndex.get());
		auto entity = m_gpu.lineStripIndex.FindEntity(uiContext.GetSelectId());
		if (entity.IsActive()) {
			pSimpleShader->SetColor(selectColor);
			pSimpleShader->DrawArray(m_gpu.lineStripIndex.drawType, entity.first, entity.num);
		}
	}

	if (m_gpu.lineLoopIndex.pPosition) {
		pSimpleShader->SetPosition(m_gpu.lineLoopIndex.pPosition.get());
		pSimpleShader->SetColor(Vector3(1.0f, 1.0f, 1.0f));
		pSimpleShader->DrawElement(m_gpu.lineLoopIndex.drawType, m_gpu.lineLoopIndex.pIndex.get());
		auto entity = m_gpu.lineLoopIndex.FindEntity(uiContext.GetSelectId());
		if (entity.IsActive()) {
			pSimpleShader->SetColor(selectColor);
			pSimpleShader->DrawArray(m_gpu.lineLoopIndex.drawType, entity.first, entity.num);
		}
	}

	if (m_ui.visibleMesh) {
		if (m_gpu.triangle.pPosition) {
			pSimpleShader->SetPosition(m_gpu.triangle.pPosition.get());
			pSimpleShader->SetColor(Vector3(1.0f, 0.0f, 0.0f));
			pSimpleShader->DrawArray(m_gpu.triangle.drawType, m_gpu.triangle.pPosition.get());
			auto entity = m_gpu.triangle.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawArray(m_gpu.triangle.drawType, entity.first, entity.num);
			}
		}

		if (m_gpu.triangleIndex.pPosition) {
			pSimpleShader->SetPosition(m_gpu.triangleIndex.pPosition.get());
			pSimpleShader->SetColor(Vector3(0.0f, 1.0f, 0.0f));
			pSimpleShader->DrawElement(m_gpu.triangleIndex.drawType, m_gpu.triangleIndex.pIndex.get());
			auto entity = m_gpu.triangleIndex.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawElement(m_gpu.triangleIndex.drawType, m_gpu.triangleIndex.pIndex.get(), entity.num, entity.first);
			}
		}
	}
}
void STEPRenderNode::ShowUI(UIContext& ui)
{
	uiContext.ui = &ui;
	uiContext.pNode = this;
	ImGui::Checkbox("VisibleBDB",&m_ui.visibleBDB);
	ImGui::Checkbox("VisibleMesh", &m_ui.visibleMesh);
	if (ImGui::TreeNode("Root")) {
		for (auto& shell : m_step->closedShell) { shell.second->ShowUI(uiContext); }
		for (auto& shell : m_step->openShell) { shell.second->ShowUI(uiContext); }
		ImGui::TreePop();
	}
}

bool STEPUIContext::IsSelect(int id) const
{
	if (!pSelect) { return false; }
	return pSelect->id == id;
}

int STEPUIContext::GetSelectId() const
{
	if (!pSelect) { return -1; }
	return pSelect->id;
}
void STEPRenderNode::UpdateData(float diff)
{
	return;
	static glm::vec3 axis = glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f));
	static float rotationSpeed = 0.01f;
	float angle = glm::radians(rotationSpeed * diff);
	auto center = GetBoundBox().Center();
	auto translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
	auto rotate = glm::rotate(glm::mat4(1.0f), angle, axis);
	auto translateBack = glm::translate(glm::mat4(1.0f), center);

	// 回転を蓄積する場合
	m_rotateMatrix = translateBack * rotate * translateToOrigin * m_rotateMatrix;
}

}
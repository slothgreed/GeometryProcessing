#include "STEPNode.h"
#include "FileUtility.h"
#include "RenderNode.h"
#include "SimpleShader.h"
#include "Polyline.h"
#include "Utility.h"
#include <functional>
namespace KI
{
#define FIND_STEP_DATA(a,b,c,d) { auto x = step.c.find(d); if(x == step.c.end()){assert(0);return a;} a.b = x->second->ToData(step);}
#define FIND_STEP_DATA2(a,b,c) { auto x = step.b.find(c); if(x == step.b.end()){assert(0);} else { a = x->second->ToData(step);}}



enum STEPEnum
{
	DOLL = -1,
	ASTERISK = -2,
};

struct STEPLine;
struct STEPPlane;
struct STEPVector;
struct STEPDirection;
struct STEPPoint;
struct STEPAxis2Placement3D;
struct STEPEdgeCurve;
struct STEPVertexPoint;
struct STEPEdgeLoop;
struct STEPFaceOuterBound;
struct STEPOrientedEdge;
struct STEPAdvancedFace;
struct STEPClosedShell;
struct STEPStruct
{
	std::unordered_map<int, STEPPoint*> points;
	std::unordered_map<int, STEPLine*> lines;
	std::unordered_map<int, STEPPlane*> planes;
	std::unordered_map<int, STEPVector*> vectors;
	std::unordered_map<int, STEPDirection*> directions;
	std::unordered_map<int, STEPEdgeCurve*> edgeCurve;
	std::unordered_map<int, STEPAxis2Placement3D*> axis2Placement3D;
	std::unordered_map<int, STEPVertexPoint*> vertexPoint;
	std::unordered_map<int, STEPEdgeLoop*> edgeLoop;
	std::unordered_map<int, STEPFaceOuterBound*> faceOuterBound;
	std::unordered_map<int, STEPOrientedEdge*> orientedEdge;
	std::unordered_map<int, STEPAdvancedFace*> advancedFace;
	std::unordered_map<int, STEPClosedShell*> closedShell;

	~STEPStruct()
	{
		for (auto& v : points) { delete v.second; }
		for (auto& v : lines) { delete v.second; }
		for (auto& v : planes) { delete v.second; }
		for (auto& v : vectors) { delete v.second; }
		for (auto& v : directions) { delete v.second; }
		for (auto& v : edgeCurve) { delete v.second; }
		for (auto& v : axis2Placement3D) { delete v.second; }
		for (auto& v : vertexPoint) { delete v.second; }
		for (auto& v : edgeLoop) { delete v.second; }
		for (auto& v : faceOuterBound) { delete v.second; }
		for (auto& v : orientedEdge) { delete v.second; }
		for (auto& v : advancedFace) { delete v.second; }
		for (auto& v : closedShell) { delete v.second; }
	}
};

struct STEPString
{
	STEPString() {};
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
		step.name = nameToValue.first;
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
};

struct STEPPoint
{
	STEPPoint() {};
	~STEPPoint() {};
	static constexpr const char* EntityName = "CARTESIAN_POINT";
	int id;
	Vector3 pos;

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPPoint();
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);
		if (!STEPString::ValueToFloat(values[0], data->pos.x)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[1], data->pos.y)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[2], data->pos.z)) { assert(0); return; }
		data->id = stepStr.id;
		step.points[data->id] = data;
	}

	Vector3 ToData(const STEPStruct& step)
	{
		return pos;
	}
};

struct STEPDirection
{
	STEPDirection() {};
	~STEPDirection() {};
	static constexpr const char* EntityName = "DIRECTION";
	int id;
	Vector3 direction;

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPDirection();
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);
		if (!STEPString::ValueToFloat(values[0], data->direction.x)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[1], data->direction.y)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[2], data->direction.z)) { assert(0); return; }
		data->id = stepStr.id;
		step.directions[data->id] = data;
	}

	Vector3 ToData(const STEPStruct& step)
	{
		return direction;
	}
};

struct STEPVector
{
	STEPVector() {};
	~STEPVector() {};
	static constexpr const char* EntityName = "VECTOR";
	int id;
	int idRef;
	float length;

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPVector();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->idRef)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[2], data->length)) { assert(0); return; }

		data->id = stepStr.id;
		step.vectors[data->id] = data;
	}

	Vector3 ToData(const STEPStruct& step)
	{
		Vector3 vector;
		FIND_STEP_DATA2(vector, directions, idRef);
		vector = glm::normalize(vector);
		vector *= length;
		return vector;
	}
};

struct STEPLine
{
	STEPLine() {};
	~STEPLine() {};
	static constexpr const char* EntityName = "LINE";
	int id;
	int beginRef;
	int endRef;

	struct Data
	{
		Vector3 begin;
		Vector3 vector;
	};

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPLine();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->beginRef)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[2], data->endRef)) { assert(0); return; }
		data->id = stepStr.id;
		step.lines[data->id] = data;
	}

	STEPLine::Data ToData(const STEPStruct& step)
	{
		STEPLine::Data data;
		FIND_STEP_DATA(data, begin, points, beginRef);
		FIND_STEP_DATA(data, vector, vectors, endRef);
		return data;
	}
};

struct STEPAxis2Placement3D
{
	STEPAxis2Placement3D() {};
	~STEPAxis2Placement3D() {};

	static constexpr const char* EntityName = "AXIS2_PLACEMENT_3D";

	int id;
	int pointRef;
	int dirRef1;
	int dirRef2;

	struct Data
	{
		Vector3 point;
		Vector3 dir1;
		Vector3 dir2;
	};

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPAxis2Placement3D();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->pointRef)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[2], data->dirRef1)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[3], data->dirRef2)) { assert(0); return; }

		data->id = stepStr.id;
		step.axis2Placement3D[data->id] = data;
	}

	STEPAxis2Placement3D::Data ToData(const STEPStruct& step)
	{
		STEPAxis2Placement3D::Data data;
		FIND_STEP_DATA(data, point, points, pointRef);
		FIND_STEP_DATA(data, dir1, directions, dirRef1);
		FIND_STEP_DATA(data, dir2, directions, dirRef2);
		return data;
	}
};

struct STEPPlane
{
	STEPPlane() {};
	~STEPPlane() {};

	static constexpr const char* EntityName = "PLANE";
	int id;
	int idRef;

	struct Data
	{
		STEPAxis2Placement3D::Data axis;
	};


	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPPlane();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->idRef)) { assert(0); return; }
		data->id = stepStr.id;
		step.planes[data->id] = data;
	}

	STEPPlane::Data ToData(const STEPStruct& step)
	{
		STEPPlane::Data data;
		step.axis2Placement3D;
		FIND_STEP_DATA(data, axis, axis2Placement3D, idRef);
		return data;
	}
};

struct STEPVertexPoint
{
	STEPVertexPoint() {};
	~STEPVertexPoint() {};

	static constexpr const char* EntityName = "VERTEX_POINT";

	int id;
	int idRef;

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPVertexPoint();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->idRef)) { assert(0); return; }
		data->id = stepStr.id;
		step.vertexPoint[data->id] = data;
	}

	Vector3 ToData(const STEPStruct& step)
	{
		Vector3 pos;
		FIND_STEP_DATA2(pos, points, idRef);
		return pos;
	}
};

struct STEPEdgeCurve
{
	STEPEdgeCurve() {};
	~STEPEdgeCurve() {};

	static constexpr const char* EntityName = "EDGE_CURVE";

	int id;
	int vertRef0;
	int vertRef1;
	int lineRef2;
	bool orient;

	struct Data
	{
		Vector3 begin;
		Vector3 end;
		STEPLine::Data line;
		bool orient;
	};


	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPEdgeCurve();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->vertRef0)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[2], data->vertRef1)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[3], data->lineRef2)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[4], data->orient)) { assert(0); return; }

		data->id = stepStr.id;
		step.edgeCurve[data->id] = data;
	}

	STEPEdgeCurve::Data ToData(const STEPStruct& step)
	{
		STEPEdgeCurve::Data data;
		FIND_STEP_DATA(data, begin, vertexPoint, vertRef0);
		FIND_STEP_DATA(data, end, vertexPoint, vertRef1);
		FIND_STEP_DATA(data, line, lines, lineRef2);

		data.orient = orient;
		return data;
	}
};

struct STEPOrientedEdge
{
	STEPOrientedEdge() {};
	~STEPOrientedEdge() {};

	static constexpr const char* EntityName = "ORIENTED_EDGE";

	int id;
	int vertRef0;
	int vertRef1;
	int edgeCurveRef2;
	bool orient;


	struct Data
	{
		Vector3 begin;
		Vector3 end;
		STEPEdgeCurve::Data edgeCurve;
		bool orient;

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
	};

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPOrientedEdge();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->vertRef0)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[2], data->vertRef1)) { assert(0); return; }
		if (!STEPString::ValueToRef(values[3], data->edgeCurveRef2)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[4], data->orient)) { assert(0); return; }
		data->id = stepStr.id;
		step.orientedEdge[data->id] = data;
	}


	STEPOrientedEdge::Data ToData(const STEPStruct& step)
	{
		STEPOrientedEdge::Data data;
		FIND_STEP_DATA(data, edgeCurve, edgeCurve, edgeCurveRef2);
		if (vertRef0 == STEPEnum::ASTERISK) {
			data.begin = data.edgeCurve.begin;
		} else {
			FIND_STEP_DATA(data, begin, vertexPoint, vertRef0);
		}

		if (vertRef1 == STEPEnum::ASTERISK) {
			data.end = data.edgeCurve.end;
		} else {
			FIND_STEP_DATA(data, end, vertexPoint, vertRef1);
		}

		data.orient = orient;
		return data;
	}

};

struct STEPEdgeLoop
{
	STEPEdgeLoop() {};
	~STEPEdgeLoop() {};

	static constexpr const char* EntityName = "EDGE_LOOP";

	int id;
	Vector<int> idRef;

	struct Data
	{
		Vector<STEPOrientedEdge::Data> orientedEdges;
		void CreatePolyline(STEPMesh& mesh) const
		{
			Polyline polyline;
			for (const auto& edge : orientedEdges) {
				if (polyline.Num() != 0) {
					if (edge.GetBegin() != polyline.Last()) {
						int a = 0;
					}
				}
				polyline.Add(edge.GetBegin());
				
				mesh.edges.push_back(edge.GetBegin());
				mesh.edges.push_back(edge.GetEnd());
				Printf::Vec3("Begin", edge.GetBegin());
				Printf::Vec3("End", edge.GetEnd());
			}
		}

	};

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPEdgeLoop();
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);

		data->idRef.resize(values.size());
		for (int i = 0; i < data->idRef.size(); i++) {
			if (!STEPString::ValueToRef(values[i], data->idRef[i])) { assert(0); return; }
		}
		data->id = stepStr.id;
		step.edgeLoop[data->id] = data;
	}

	STEPEdgeLoop::Data ToData(const STEPStruct& step)
	{
		STEPEdgeLoop::Data data;
		data.orientedEdges.resize(idRef.size());
		for (auto i = 0; i < idRef.size(); i++) {
			FIND_STEP_DATA(data, orientedEdges[i], orientedEdge, idRef[i]);
		}
		return data;
	}

};

struct STEPFaceOuterBound
{
	STEPFaceOuterBound() {};
	~STEPFaceOuterBound() {};

	static constexpr const char* EntityName = "FACE_OUTER_BOUND";

	int id;
	int idRef0;
	bool orient;

	struct Data
	{
		STEPEdgeLoop::Data edgeLoop;
		bool orient;

	};


	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPFaceOuterBound();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->idRef0)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[2], data->orient)) { assert(0); return; }
		data->id = stepStr.id;
		step.faceOuterBound[data->id] = data;
	}

	STEPFaceOuterBound::Data ToData(const STEPStruct& step)
	{
		STEPFaceOuterBound::Data data;
		FIND_STEP_DATA(data, edgeLoop, edgeLoop, idRef0);
		data.orient = orient;
		return data;
	}

	static Polyline CreateLineStrip(const STEPFaceOuterBound::Data& data)
	{
		Vector<Vector3> strip;
		if (data.edgeLoop.orientedEdges.size() == 0) { return Vector<Vector3>(); }
		const auto& orientedEdges = data.edgeLoop.orientedEdges;
		for (int i = 0; i < orientedEdges.size(); i++) {
			if (orientedEdges[i].begin != orientedEdges[i - 1].end) {
				assert(0);
			}

			strip.push_back(orientedEdges[i].begin);
		}

		strip.push_back(orientedEdges[orientedEdges.size() - 1].end);

		return Polyline(std::move(strip));
	}

};

struct STEPAdvancedFace
{
	STEPAdvancedFace() {};
	~STEPAdvancedFace() {};

	static constexpr const char* EntityName = "ADVANCED_FACE";
	int id;
	Vector<int> faceRef0;
	int planeRef1;
	bool orient;

	struct Data
	{
		Vector<STEPFaceOuterBound::Data> faceOuterBound;
		STEPPlane::Data plane;
		bool orient;
	};
	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPAdvancedFace();
		auto values = STEPString::SplitValue(stepStr.value);
		auto faces = STEPString::SplitValue(values[1]);
		data->faceRef0.resize(faces.size());
		for (int i = 0; i < faces.size(); i++) {
			if (!STEPString::ValueToRef(faces[i], data->faceRef0[i])) { assert(0); return; }
		}

		if (!STEPString::ValueToRef(values[2], data->planeRef1)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[3], data->orient)) { assert(0); return; }
		data->id = stepStr.id;
		step.advancedFace[data->id] = data;
	}

	STEPAdvancedFace::Data ToData(const STEPStruct& step)
	{
		STEPAdvancedFace::Data data;

		data.faceOuterBound.resize(faceRef0.size());
		for (auto i = 0; i < faceRef0.size(); i++) {
			FIND_STEP_DATA(data, faceOuterBound[i], faceOuterBound, faceRef0[i]);
		}
		FIND_STEP_DATA(data, plane, planes, planeRef1);
		data.orient = orient;
		return data;
	}

};

struct STEPClosedShell
{
	STEPClosedShell() {};
	~STEPClosedShell() {};

	static constexpr const char* EntityName = "CLOSED_SHELL";

	int id;
	Vector<int> faceRef;

	struct Data
	{
		Vector<STEPAdvancedFace::Data> advancedFace;
	};

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPClosedShell();
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);
		data->faceRef.resize(values.size());
		for (int i = 0; i < values.size(); i++) {
			if (!STEPString::ValueToRef(values[i], data->faceRef[i])) { assert(0); return; }
		}
		data->id = stepStr.id;
		step.closedShell[data->id] = data;
	}

	STEPClosedShell::Data ToData(const STEPStruct& step)
	{
		STEPClosedShell::Data data;

		data.advancedFace.resize(faceRef.size());
		for (auto i = 0; i < faceRef.size(); i++) {
			FIND_STEP_DATA(data, advancedFace[i], advancedFace, faceRef[i]);
		}
		return data;
	}

	STEPMesh CreateMesh(const STEPStruct& step)
	{
		STEPMesh mesh;
		auto data = ToData(step);
		for (const auto& advancedFace : data.advancedFace) {
			for (const auto& face : advancedFace.faceOuterBound) {
				for (const auto& edge : face.edgeLoop.orientedEdges) {
					face.edgeLoop.CreatePolyline(mesh);
				}
			}
		}

		return mesh;
	}
};

void NotDefineEntity(const String& str)
{
	// 形状に不必要なEntity
	// マテリアル情報も含まれる。
	static const std::unordered_set<std::string> handlers = {
		"PRODUCT",
		"PRODUCT_CONTEXT",
		"PRODUCT_DEFINITION",
		"PRODUCT_DEFINITION_CONTEXT",
		"PRODUCT_DEFINITION_SHAPE",
		"SHAPE_REPRESENTATION_RELATIONSHIP",
		"STYLED_ITEM",
		"PRESENTATION_STYLE_ASSIGNMENT",
		"DIMENSIONAL_EXPONENTS",
		"SURFACE_STYLE_USAGE",
		"SURFACE_SIDE_STYLE",	// 裏面表面両面
		"SURFACE_STYLE_FILL_AREA", // 塗りつぶし
		"FILL_AREA_STYLE",
		"FILL_AREA_STYLE_COLOUR", // マテリアル
		"CALENDAR_DATE",
		"DATE_AND_TIME",
		"LOCAL_TIME",
		"COORDINATED_UNIVERSAL_TIME_OFFSET",
		"PRODUCT_RELATED_PRODUCT_CATEGORY",
		"UNCERTAINTY_MEASURE_WITH_UNIT",
		"PRODUCT_DEFINITION_FORMATION_WITH_SPECIFIED_SOURCE",
		"SHAPE_DEFINITION_REPRESENTATION",
		"PRODUCT_CATEGORY_RELATIONSHIP",
		"LENGTH_MEASURE_WITH_UNIT",
		"PLANE_ANGLE_MEASURE_WITH_UNIT", // 角度orラジアン
		"APPLICATION_PROTOCOL_DEFINITION",
		"DRAUGHTING_PRE_DEFINED_COLOUR"
	};

	"ADVANCED_BREP_SHAPE_REPRESENTATION";
	"MANIFOLD_SOLID_BREP";

	for (const auto& key : handlers) {
		if (StringUtility::Contains(str, key)) {
			return;
		}
	}

	// 検討していないエンティティ
	printf("%s\n", str.data());
}




RenderNode* STEPLoader::Load(const String& name)
{
	auto extension = FileUtility::GetExtension(name);
	if (!(extension == ".step" || extension == ".stp")) { return nullptr; }
	Vector<String> contents;
	if (!FileUtility::Load(name, contents)) {
		return nullptr;
	}

	int dataIndex = -1;
	for (int i = 0; i < contents.size(); i++) {
		if (contents[i] == "DATA;") {
			dataIndex = i;
			break;
		}
	}

	if (dataIndex == -1) { return nullptr; }

	STEPStruct step;

	for (int i = dataIndex + 1; contents.size(); i++) {
		if (contents[i] == "ENDSEC;")break;
		auto stepStr = STEPString::Create(contents[i]);
		if (StringUtility::Contains(stepStr.name, STEPPoint::EntityName)) { STEPPoint::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPDirection::EntityName)) { STEPDirection::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPVector::EntityName)) { STEPVector::Fetch(step, stepStr);}
		else if (StringUtility::Contains(stepStr.name, STEPPlane::EntityName)) { STEPPlane::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPLine::EntityName)) { STEPLine::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPAxis2Placement3D::EntityName)) { STEPAxis2Placement3D::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPEdgeCurve::EntityName)) { STEPEdgeCurve::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPVertexPoint::EntityName)) { STEPVertexPoint::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPEdgeLoop::EntityName)) { STEPEdgeLoop::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPFaceOuterBound::EntityName)) { STEPFaceOuterBound::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPOrientedEdge::EntityName)) { STEPOrientedEdge::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPAdvancedFace::EntityName)) { STEPAdvancedFace::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPClosedShell::EntityName)) { STEPClosedShell::Fetch(step, stepStr); }
		else { NotDefineEntity(contents[i]); }
	}

	STEPClosedShell::Data data;
	Vector<STEPMesh> meshs;
	for (const auto& face : step.closedShell) {
		data = face.second->ToData(step);
		meshs.push_back(face.second->CreateMesh(step));
	}

	STEPRenderNode* pRenderNode = new STEPRenderNode(name);
	pRenderNode->SetMesh(std::move(meshs));

	return pRenderNode;

}

void STEPRenderNode::BuildGLResource()
{
	if (m_mesh.size() == 0) { return; }
	if (m_gpu.pTriangles != nullptr &&
		m_gpu.pEdges != nullptr &&
		m_gpu.pVertexs != nullptr) {
		return;
	}

	size_t triangleNum = 0;
	size_t edgeNum = 0;
	size_t vertexNum = 0;
	for (const auto& mesh : m_mesh) {
		triangleNum = mesh.triangels.size();
		edgeNum = mesh.edges.size();
		vertexNum = mesh.vertexs.size();
	}

	if (triangleNum != 0) {
		m_gpu.pTriangles = std::make_unique<GLBuffer>();
		m_gpu.pTriangles->Create(DATA_FLOAT, triangleNum, sizeof(Vector3), nullptr);
	}
	if (edgeNum != 0) {
		m_gpu.pEdges = std::make_unique<GLBuffer>();
		m_gpu.pEdges->Create(DATA_FLOAT, edgeNum, sizeof(Vector3), nullptr);
	}
	if (vertexNum != 0) {
		m_gpu.pVertexs = std::make_unique<GLBuffer>();
		m_gpu.pVertexs->Create(DATA_FLOAT, vertexNum, sizeof(Vector3), nullptr);
	}
	size_t triangleOffset = 0;
	size_t edgeOffset = 0;
	size_t vertexOffset = 0;

	for (const auto& mesh : m_mesh) {
		if (mesh.triangels.size() != 0) {
			m_gpu.pTriangles->BufferSubData(triangleOffset, mesh.triangels);
			triangleOffset = mesh.triangels.size();
		}

		if (mesh.edges.size() != 0) {
			m_gpu.pEdges->BufferSubData(edgeOffset, mesh.edges);
			edgeOffset = mesh.edges.size();
		}
		
		if (mesh.vertexs.size() != 0) {
			m_gpu.pVertexs->BufferSubData(vertexOffset, mesh.vertexs);
			vertexOffset = mesh.vertexs.size();
		}
	}
}
void STEPRenderNode::DrawNode(const DrawContext& context)
{
	BuildGLResource();
	auto pResource = context.pResource;
	auto pSimpleShader = pResource->GetShaderTable()->GetSimpleShader();
	pSimpleShader->Use();
	pSimpleShader->SetPosition(m_gpu.pEdges.get());
	pSimpleShader->SetCamera(pResource->GetCameraBuffer());
	pSimpleShader->SetModel(GetMatrix());
	pSimpleShader->SetColor(Vector3(0.7f, 0.7f, 1.0f));
	pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
	pSimpleShader->DrawArray(GL_LINES, m_gpu.pEdges.get());
}
void STEPRenderNode::ShowUI(UIContext& ui)
{

}


}
#include "STEPNode.h"
#include "FileUtility.h"
#include "RenderNode.h"
#include "SimpleShader.h"
#include "Polyline.h"
#include "Utility.h"
#include "Primitives.h"
#include <functional>
namespace KI
{
#define FIND_SET_DATA(a,b,c,d) { auto x = step.c.find(d); if(x != step.c.end()) { a.b = x->second->ToData(step);}}
#define FIND_SET_DATA2(a,b,c) { auto x = step.b.find(c); if(x == step.b.end()) {a = x->second->ToData(step);}}

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
struct STEPFaceBound;
struct STEPOrientedEdge;
struct STEPAdvancedFace;
struct STEPClosedShell;
struct STEPOpenShell;
struct STEPCircle;
struct STEPCylinderSurface;
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
	std::unordered_map<int, STEPEdgeLoop*> edgeLoop;
	std::unordered_map<int, STEPFaceOuterBound*> faceOuterBound;
	std::unordered_map<int, STEPFaceBound*> faceBound;
	std::unordered_map<int, STEPOrientedEdge*> orientedEdge;
	std::unordered_map<int, STEPAdvancedFace*> advancedFace;
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
		for (auto& v : edgeLoop) { delete v.second; }
		for (auto& v : faceOuterBound) { delete v.second; }
		for (auto& v : faceBound) { delete v.second; }
		for (auto& v : orientedEdge) { delete v.second; }
		for (auto& v : advancedFace) { delete v.second; }
		for (auto& v : closedShell) { delete v.second; }
		for (auto& v : openShell) { delete v.second; }
	}
};

template <typename Struct, typename Value>
bool FindSetData(const STEPStruct& step, const std::unordered_map<int, Struct*>& container, int key, Value& outValue)
{
	auto it = container.find(key);
	if (it == container.end()) {
		DebugPrintf::StringStr("NotFound", Struct::EntityName); DebugPrintf::NewLine();
		return false;
	}
	outValue = it->second->ToData(step);
	return true;
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
		step.name = nameToValue.first;
		step.value = '(' + nameToValue.second;
		return step;
	}

	static Vector<String> SplitValue(const String& value)
	{	
		Vector<String> result;
		size_t start = 0;
		int depth = 0;  // ���ʂ̐[��
		for (size_t i = 0; i < value.size(); ++i) {
			char ch = value[i];

			// ���ʂ̊J�n
			if (ch == '(') {
				if (depth == 0) start = i + 1;  // �ŏ��� '(' �̌ォ��J�n
				++depth;
				// ���ʂ̏I��
			} else if (ch == ')') {
				--depth;
				if (depth == 0) {
					// �ŊO�̊��ʂ������烊�X�g�Ƃ��Ēǉ�
					result.push_back(value.substr(start, i - start));
				}
			}
			// ���ʓ��̃J���}�ŋ�؂�
			else if (depth == 1 && ch == ',' && (i == start || value[i - 1] != '\\')) {
				// �J���}����؂�Ƃ��ĕ�����i�G�X�P�[�v�����������j
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

		// �擪�� '#' �̏ꍇ�͏���
		auto numStr = (str[0] == '#') ? str.substr(1) : str;

		// atoi() ���g���Đ����ϊ�

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

	// �������h���L��������
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
	STEPPoint():id(-1){};
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
	STEPDirection():id(-1) {};
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
	STEPVector() :id(-1), idRef(-1), length(0.0f) {};
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
		FindSetData(step, step.directions, idRef, vector);
		vector = glm::normalize(vector);
		vector *= length;
		return vector;
	}
};

struct STEPLine
{
	STEPLine():id(-1),beginRef(-1),endRef(-1) {};
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
		FindSetData(step, step.points, beginRef, data.begin);
		FindSetData(step, step.vectors, endRef, data.vector);
		return data;
	}
};

struct STEPAxis2Placement3D
{
	STEPAxis2Placement3D() :id(-1), pointRef(-1), dirRef1(-1), dirRef2(-1) {};
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

		Vector3 Normal() const
		{
			return dir1;
		}

		Vector3 U() const
		{
			return dir2;
		}

		Vector3 V() const
		{
			return glm::cross(dir1, dir2);
		}
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
		FindSetData(step, step.points, pointRef, data.point);
		FindSetData(step, step.directions, dirRef1, data.dir1);
		FindSetData(step, step.directions, dirRef2, data.dir2);

		return data;
	}
};


struct STEPCircle
{
	STEPCircle() :id(-1), axisRef(-1), rad(0.0f) {};
	~STEPCircle() {};
	static constexpr const char* EntityName = "CIRCLE";
	int id;
	int axisRef;
	float rad;

	struct Data
	{
		Data() :rad(0.0f) {}
		STEPAxis2Placement3D::Data axis;
		float rad;

		Vector<Vector3> GetCircleEdge() const
		{
			const int CIRCLE_POINT_NUM = 36;
			auto v = glm::cross(axis.dir1, axis.dir2);
			return Circle::CreateLine(rad, CIRCLE_POINT_NUM, axis.dir2, v, axis.point);
		}

		void CreateEdges(STEPMesh& mesh) const
		{
			auto lines = GetCircleEdge();
			mesh.edges.insert(mesh.edges.begin(), lines.begin(), lines.end());
		}
	};

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPCircle();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->axisRef)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[2], data->rad)) { assert(0); return; }
		data->id = stepStr.id;
		step.circles[data->id] = data;
	}

	STEPCircle::Data ToData(const STEPStruct& step)
	{
		STEPCircle::Data data;
		FindSetData(step, step.axis2Placement3D, axisRef, data.axis);
		data.rad = rad;
		return data;
	}

};

struct STEPCylinderSurface
{
	STEPCylinderSurface() :id(-1), axisRef(-1), rad(0.0f) {};
	~STEPCylinderSurface() {};
	static constexpr const char* EntityName = "CYLINDRICAL_SURFACE";
	int id;
	int axisRef;
	float rad;

	struct Data
	{
		STEPAxis2Placement3D::Data axis;
		float rad;
	};

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPCylinderSurface();
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->axisRef)) { assert(0); return; }
		if (!STEPString::ValueToFloat(values[2], data->rad)) { assert(0); return; }
		data->id = stepStr.id;
		step.cylinderSurface[data->id] = data;
	}

	STEPCylinderSurface::Data ToData(const STEPStruct& step)
	{
		STEPCylinderSurface::Data data;
		FindSetData(step, step.axis2Placement3D, axisRef, data.axis);
		data.rad = rad;
		return data;
	}
};

struct STEPPlane
{
	STEPPlane() :id(0), idRef(0) {};
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
		FindSetData(step, step.axis2Placement3D, idRef, data.axis);
		return data;
	}
};

struct STEPVertexPoint
{
	STEPVertexPoint() : id(-1), idRef(-1) {};
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
		FindSetData(step, step.points, idRef, pos);
		return pos;
	}
};

struct STEPEdgeCurve
{
	STEPEdgeCurve() :id(-1), vertRef0(-1), vertRef1(-1), lineRef2(-1), orient(true) {};
	~STEPEdgeCurve() {};

	static constexpr const char* EntityName = "EDGE_CURVE";

	int id;
	int vertRef0;
	int vertRef1;
	int lineRef2;
	bool orient;
	enum class CurveType
	{
		Line,
		Circle,
	};

	struct Data
	{
		Data() :orient(true), type(CurveType::Line) {}
		Vector3 begin;
		Vector3 end;
		STEPLine::Data line;
		STEPCircle::Data circle;
		bool orient;
		CurveType type;
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
		FindSetData(step, step.vertexPoint, vertRef0, data.begin);
		FindSetData(step, step.vertexPoint, vertRef1, data.end);
		if (FindSetData(step, step.lines, lineRef2, data.line)) {
			data.type = CurveType::Line;
		} else if (FindSetData(step, step.circles, lineRef2, data.circle)) {
			data.type = CurveType::Circle;
		}

		data.orient = orient;
		return data;
	}
};

struct STEPOrientedEdge
{
	STEPOrientedEdge() :id(-1), vertRef0(-1), vertRef1(-1), edgeCurveRef2(-1), orient(true) {};
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

		bool IsLine() const
		{
			return edgeCurve.type == STEPEdgeCurve::CurveType::Line;
		}

		bool IsCircle() const
		{
			return edgeCurve.type == STEPEdgeCurve::CurveType::Circle;
		}

		Vector3 GetBegin() const
		{
			if (orient) {
				return begin;
			} else {
				return end;
			}
		}

		Vector<Vector3> GetCircleEdge() const
		{
			if (!IsCircle()) {
				assert(0);
				return Vector<Vector3>();
			}

			return edgeCurve.circle.GetCircleEdge();
		}

		void CreateEdges(STEPMesh& mesh) const
		{
			if (IsLine()) {
				if (orient) {
					mesh.edges.push_back(begin);
					mesh.edges.push_back(end);
				} else {
					mesh.edges.push_back(end);
					mesh.edges.push_back(begin);
				}
			} else if(IsCircle()){
				edgeCurve.circle.CreateEdges(mesh);
			}
		}

	};

	static void Printf(const STEPOrientedEdge::Data& data)
	{
		DebugPrintf::Bool("Orient", data.orient);
		DebugPrintf::Vec3("Begin", data.begin);
		DebugPrintf::Vec3("End", data.end);
		DebugPrintf::NewLine();
	}

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
		FindSetData(step, step.edgeCurve, edgeCurveRef2, data.edgeCurve);
		if (vertRef0 == STEPEnum::ASTERISK) {
			data.begin = data.edgeCurve.begin;
		} else {
			FindSetData(step, step.vertexPoint, vertRef0, data.begin);
		}

		if (vertRef1 == STEPEnum::ASTERISK) {
			data.end = data.edgeCurve.end;
		} else {
			FindSetData(step, step.vertexPoint, vertRef1, data.end);
		}

		data.orient = orient;
		return data;
	}




};

struct STEPEdgeLoop
{
	STEPEdgeLoop() :id(-1) {};
	~STEPEdgeLoop() {};

	static constexpr const char* EntityName = "EDGE_LOOP";

	int id;
	Vector<int> idRef;

	struct Data
	{
		Vector<STEPOrientedEdge::Data> orientedEdges;
		Polyline CreatePolyline() const
		{
			Polyline polyline;
			for (const auto& edge : orientedEdges) {
				if (edge.IsLine()) {
					polyline.Add(edge.GetBegin());
				} else if (edge.IsCircle()) {
					polyline.AddCircle(edge.GetCircleEdge());
				}
			}

			return polyline;
		}

		void CreatePlane(STEPMesh& mesh, bool orient) const
		{
			for (const auto& edge : orientedEdges) {
				edge.CreateEdges(mesh);
			}
			auto polyline = CreatePolyline();
			auto triangle = polyline.CreateTrianglePoints(orient);
			mesh.triangels.insert(mesh.triangels.end(), triangle.begin(), triangle.end());
		}

		void CreateCylinder(STEPMesh& mesh, bool orient, const STEPCylinderSurface::Data& cylinder) const
		{
			std::array<const STEPOrientedEdge::Data*, 2> topBottom{};
			int i = 0;
			for (const auto& edge : orientedEdges) {
				if (edge.IsCircle()) { topBottom[i++] = &edge; if (i == 2) break; }
			}
			Vector3 top, bottom;
			float height = 0.0f;
			if (glm::dot(cylinder.axis.Normal(), topBottom[1]->GetBegin() - topBottom[0]->GetBegin()) > 0.0f) {
				top = topBottom[1]->GetBegin();
				bottom = topBottom[0]->GetBegin();
				height = -glm::length(top - bottom);
			} else {
				top = topBottom[0]->GetBegin();
				bottom = topBottom[1]->GetBegin();
				height = glm::length(top - bottom);
			}
			auto cylinderMesh = Cylinder::CreateMeshs(cylinder.axis.point, top - bottom, cylinder.rad, height, 32, 32);
			mesh.edges.insert(mesh.edges.end(), cylinderMesh.edges.begin(), cylinderMesh.edges.end());
			mesh.triangels.insert(mesh.triangels.end(), cylinderMesh.triangles.begin(), cylinderMesh.triangles.end());
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
			FindSetData(step, step.orientedEdge, idRef[i], data.orientedEdges[i]);
		}
		return data;
	}

};

struct STEPFace
{
	STEPFace() :id(-1), idRef0(-1), orient(true) {};
	~STEPFace() {};

	int id;
	int idRef0;
	bool orient;

	struct Data
	{
		Data() :orient(true) {}
		STEPEdgeLoop::Data edgeLoop;
		bool orient;

		void CreatePlane(STEPMesh& mesh) const
		{
			edgeLoop.CreatePlane(mesh, orient);
		}

		void CreateCylinder(STEPMesh& mesh, const STEPCylinderSurface::Data& cylinder) const
		{
			edgeLoop.CreateCylinder(mesh, orient, cylinder);
		}
	};

	static void Fetch(STEPStruct& step, const STEPString& stepStr, STEPFace* data)
	{
		auto values = STEPString::SplitValue(stepStr.value);
		if (!STEPString::ValueToRef(values[1], data->idRef0)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[2], data->orient)) { assert(0); return; }
		data->id = stepStr.id;
	}

	void ToData(const STEPStruct& step, STEPFace::Data* data)
	{
		FindSetData(step, step.edgeLoop, idRef0, data->edgeLoop);
		data->orient = orient;
	}
};

struct STEPFaceOuterBound : public STEPFace
{
	STEPFaceOuterBound() {};
	~STEPFaceOuterBound() {};

	static constexpr const char* EntityName = "FACE_OUTER_BOUND";

	struct Data : public STEPFace::Data	{};


	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPFaceOuterBound();
		STEPFace::Fetch(step, stepStr, data);
		step.faceOuterBound[data->id] = data;
	}

	STEPFaceOuterBound::Data ToData(const STEPStruct& step)
	{
		STEPFaceOuterBound::Data data;
		STEPFace::ToData(step, &data);
		return data;
	}
};

struct STEPFaceBound : public STEPFace
{
	STEPFaceBound()  {};
	~STEPFaceBound() {};

	static constexpr const char* EntityName = "FACE_BOUND";

	struct Data : public STEPFace::Data {};

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPFaceBound();
		STEPFace::Fetch(step, stepStr, data);
		step.faceBound[data->id] = data;
	}

	STEPFaceBound::Data ToData(const STEPStruct& step)
	{
		STEPFaceBound::Data data;
		STEPFace::ToData(step, &data);
		return data;

	}
};

struct STEPAdvancedFace
{
	STEPAdvancedFace() :id(-1), geomRef1(-1), orient(true) {};
	~STEPAdvancedFace() {};

	static constexpr const char* EntityName = "ADVANCED_FACE";
	int id;
	Vector<int> faceRef0;
	int geomRef1;
	bool orient;

	enum class GeomType
	{
		Plane,
		Cylinder,
	};

	struct Data
	{
		Data()
			: type(GeomType::Plane)
			, orient(true)
		{
		}
		Vector<STEPFaceBound::Data> faceBound;
		Vector<STEPFaceOuterBound::Data> faceOuterBound;
		STEPPlane::Data plane;
		STEPCylinderSurface::Data cylinder;
		bool orient;
		GeomType type;

		void CreateMesh(STEPMesh& mesh) const
		{
			if (type == GeomType::Plane) {
				for (const auto& face : faceBound) {
					face.CreatePlane(mesh);
				}

				for (const auto& face : faceOuterBound) {
					face.CreatePlane(mesh);
				}
			} else if (type == GeomType::Cylinder) {
				for (const auto& face : faceBound) {
					face.CreateCylinder(mesh, cylinder);
				}

				for (const auto& face : faceOuterBound) {
					face.CreateCylinder(mesh, cylinder);
				}
			}
		}
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

		if (!STEPString::ValueToRef(values[2], data->geomRef1)) { assert(0); return; }
		if (!STEPString::ValueToBool(values[3], data->orient)) { assert(0); return; }
		data->id = stepStr.id;
		step.advancedFace[data->id] = data;
	}

	STEPAdvancedFace::Data ToData(const STEPStruct& step)
	{
		STEPAdvancedFace::Data data;

		for (auto i = 0; i < faceRef0.size(); i++) {
			STEPFaceOuterBound::Data faceOuterBound;
			if (FindSetData(step, step.faceOuterBound, faceRef0[i], faceOuterBound)) { data.faceOuterBound.push_back(std::move(faceOuterBound)); }
			STEPFaceBound::Data faceBound;
			if (FindSetData(step, step.faceBound, faceRef0[i], faceBound)) { data.faceBound.push_back(std::move(faceBound)); }
		}
		if (FindSetData(step, step.planes, geomRef1, data.plane)) { data.type = GeomType::Plane; }
		if (FindSetData(step, step.cylinderSurface, geomRef1, data.cylinder)) { data.type = GeomType::Cylinder; }

		data.orient = orient;
		return data;
	}

};

struct STEPShell
{
	STEPShell() :id(-1) {};
	~STEPShell() {};
	int id;
	Vector<int> faceRef;


	struct Data
	{
		Vector<STEPAdvancedFace::Data> advancedFace;
	};

	static void Fetch(STEPStruct& step, const STEPString& stepStr, STEPShell* pShell)
	{
		auto values = STEPString::SplitValue(stepStr.value);
		values = STEPString::SplitValue(values[1]);
		pShell->faceRef.resize(values.size());
		for (int i = 0; i < values.size(); i++) {
			if (!STEPString::ValueToRef(values[i], pShell->faceRef[i])) { assert(0); return; }
		}
		pShell->id = stepStr.id;
	}

	void ToData(const STEPStruct& step, STEPShell::Data* data)
	{
		data->advancedFace.resize(faceRef.size());
		for (auto i = 0; i < faceRef.size(); i++) {
			FindSetData(step, step.advancedFace, faceRef[i], data->advancedFace[i]);
		}
	}

	void CreateMesh(const STEPShell::Data& data, STEPMesh& mesh)
	{
		for (const auto& advancedFace : data.advancedFace) {
			advancedFace.CreateMesh(mesh);
		}

	}
};
struct STEPClosedShell : public STEPShell
{
	STEPClosedShell() {};
	~STEPClosedShell() {};

	static constexpr const char* EntityName = "CLOSED_SHELL";

	struct Data : STEPShell::Data {};

	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPClosedShell();
		STEPShell::Fetch(step, stepStr, data);
		step.closedShell[data->id] = data;
	}

	STEPClosedShell::Data ToData(const STEPStruct& step)
	{
		STEPClosedShell::Data data;
		STEPShell::ToData(step, &data);
		return data;
	}

	STEPMesh CreateMesh(const STEPStruct& step)
	{
		STEPMesh mesh;
		STEPShell::CreateMesh(ToData(step), mesh);
		return mesh;
	}
};

struct STEPOpenShell : public STEPShell
{
	STEPOpenShell() {};
	~STEPOpenShell() {};

	static constexpr const char* EntityName = "OPEN_SHELL";
	struct Data : STEPShell::Data {};


	static void Fetch(STEPStruct& step, const STEPString& stepStr)
	{
		auto data = new STEPOpenShell();
		STEPShell::Fetch(step, stepStr, data);
		step.openShell[data->id] = data;
	}

	STEPOpenShell::Data ToData(const STEPStruct& step)
	{
		STEPOpenShell::Data data;
		STEPShell::ToData(step, &data);
		return data;
	}

	STEPMesh CreateMesh(const STEPStruct& step)
	{
		STEPMesh mesh;
		STEPShell::CreateMesh(ToData(step), mesh);
		return mesh;
	}
};

void NotDefineEntity(const String& str)
{
	// �`��ɕs�K�v��Entity
	// �}�e���A�������܂܂��B
	static const std::unordered_set<std::string> handlers = {
		"APPLICATION_CONTEXT",
		"APPLICATION_PROTOCOL_DEFINITION",
		"APPROVAL",
		"APPROVAL_DATE_TIME",
		"APPROVAL_ROLE",
		"APPROVAL_STATUS",
		"CALENDAR_DATE",
		"CC_DESIGN_APPROVAL",
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
		"FILL_AREA_STYLE_COLOUR", // �}�e���A��
		"LENGTH_MEASURE_WITH_UNIT",
		"LOCAL_TIME",
		"MECHANICAL_CONTEXT",
		"ORGANIZATION",
		"PERSON",
		"PERSON_AND_ORGANIZATION",
		"PLANE_ANGLE_MEASURE_WITH_UNIT", // �p�xor���W�A��
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
		"SHAPE_DEFINITION_REPRESENTATION",
		"SHAPE_REPRESENTATION_RELATIONSHIP",
		"STYLED_ITEM",
		"SURFACE_SIDE_STYLE",    // ���ʕ\�ʗ���
		"SURFACE_STYLE_FILL_AREA", // �h��Ԃ�
		"SURFACE_STYLE_USAGE",
		"UNCERTAINTY_MEASURE_WITH_UNIT"
	};

	"ADVANCED_BREP_SHAPE_REPRESENTATION";
	"MANIFOLD_SOLID_BREP";

	for (const auto& key : handlers) {
		if (StringUtility::Contains(str, key)) {
			return;
		}
	}

	// �������Ă��Ȃ��G���e�B�e�B
	printf("%s\n", str.data());
}


RenderNode* STEPLoader::CreateRenderNode(const String& name, const STEPStruct& step)
{
	Vector<STEPMesh> meshs;
	for (const auto& face : step.closedShell) {
		meshs.push_back(face.second->CreateMesh(step));
	}

	for (const auto& face : step.openShell) {
		meshs.push_back(face.second->CreateMesh(step));
	}

	STEPRenderNode* pRenderNode = new STEPRenderNode(name);
	pRenderNode->SetMesh(std::move(meshs));

	return pRenderNode;
}

RenderNode* STEPLoader::Load(const String& name, bool saveOriginal)
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

	STEPStruct step;

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
		if (StringUtility::Contains(stepStr.name, STEPPoint::EntityName)) { STEPPoint::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPDirection::EntityName)) { STEPDirection::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPVector::EntityName)) { STEPVector::Fetch(step, stepStr);}
		else if (StringUtility::Contains(stepStr.name, STEPPlane::EntityName)) { STEPPlane::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPLine::EntityName)) { STEPLine::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPAxis2Placement3D::EntityName)) { STEPAxis2Placement3D::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPEdgeCurve::EntityName)) { STEPEdgeCurve::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPVertexPoint::EntityName)) { STEPVertexPoint::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPEdgeLoop::EntityName)) { STEPEdgeLoop::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPFaceBound::EntityName)) { STEPFaceBound::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPFaceOuterBound::EntityName)) { STEPFaceOuterBound::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPOrientedEdge::EntityName)) { STEPOrientedEdge::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPAdvancedFace::EntityName)) { STEPAdvancedFace::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPClosedShell::EntityName)) { STEPClosedShell::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPOpenShell::EntityName)) { STEPOpenShell::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPCircle::EntityName)) { STEPCircle::Fetch(step, stepStr); }
		else if (StringUtility::Contains(stepStr.name, STEPCylinderSurface::EntityName)) { STEPCylinderSurface::Fetch(step, stepStr); }
		else { NotDefineEntity(content); writeEntity = false; }

		if (writeEntity && saveOriginal) {
			writer.Write(content, true);
		}
	}

	if (saveOriginal) {
		writer.Close();
	}

	return CreateRenderNode(name, step);
}


void STEPRenderNode::BuildGLResource()
{
	if (m_mesh.size() == 0) { return; }
	if (m_gpu.pTriangles != nullptr ||
		m_gpu.pEdges != nullptr ||
		m_gpu.pVertexs != nullptr) {
		return;
	}
	
	bool makePolygon = false;
	if(makePolygon)
	{
		Vector<Vector3> triangles;
		Vector<Vector3> edges;
		for (const auto& mesh : m_mesh) {
			for (const auto& polyline : mesh.polylines) {
				auto polyTri = polyline.CreateTrianglePoints(true);
				auto polyEdge = polyline.CreateLinePoints();
				triangles.insert(triangles.end(), polyTri.begin(), polyTri.end());
				edges.insert(edges.end(), polyEdge.begin(), polyEdge.end());
			}
		}

		if (triangles.size() != 0) {
			m_gpu.pTriangles = std::make_unique<GLBuffer>();
			m_gpu.pTriangles->Create(DATA_FLOAT, triangles.size(), sizeof(Vector3), triangles.data());
		}
		if (edges.size() != 0) {
			m_gpu.pEdges = std::make_unique<GLBuffer>();
			m_gpu.pEdges->Create(DATA_FLOAT, edges.size(), sizeof(Vector3), edges.data());
		}
	} else {
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
}
void STEPRenderNode::DrawNode(const DrawContext& context)
{
	BuildGLResource();
	auto pResource = context.pResource;
	auto pSimpleShader = pResource->GetShaderTable()->GetSimpleShader();
	if (m_gpu.pEdges || m_gpu.pTriangles) {
		pSimpleShader->Use();
		pSimpleShader->SetCamera(pResource->GetCameraBuffer());
		pSimpleShader->SetModel(GetMatrix());
		pSimpleShader->SetColor(Vector3(0.7f, 0.7f, 1.0f));
		pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
		if (m_gpu.pEdges) {
			pSimpleShader->SetPosition(m_gpu.pEdges.get());
			pSimpleShader->DrawArray(GL_LINES, m_gpu.pEdges.get());
		}


		if (m_gpu.pTriangles) {
			pSimpleShader->SetColor(Vector3(1.0f, 0.0f, 0.0f));
			pSimpleShader->SetPosition(m_gpu.pTriangles.get());
			pSimpleShader->DrawArray(GL_TRIANGLES, m_gpu.pTriangles.get());
		}

	}
}
void STEPRenderNode::ShowUI(UIContext& ui)
{

}


}
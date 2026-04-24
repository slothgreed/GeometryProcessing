#include "STEPEntity.h"
#include "FileUtility.h"
#include "Utility.h"
#include "KIMath.h"
#include "Primitives.h"
#include "STEPNode.h"
#include "STEPUtility.h"
namespace KI
{

STEPString STEPString::Create(const String& str)
{
	STEPString step;
	auto idToEntity = StringUtility::SplitAtFirst(str, '=');
	step.id = StringUtility::ToInt(StringUtility::Remove(idToEntity.first, '#'));
	auto nameToValue = StringUtility::SplitAtFirst(idToEntity.second, '(');
	step.entity.name = StringUtility::TrimWhiteSpace(nameToValue.first);
	step.entity.value = '(' + nameToValue.second;
	if (step.entity.name.empty()) {
		step.multiEntity = CreateMultiEntity(step.entity.value);
	}
	return step;
}

ESTEPEntityType STEPString::GetMultiEntityType() const
{
	if (multiEntity.empty()) { return ESTEPEntityType::ESTEPNone; }
	for (const auto& entity : multiEntity) {
		if (StringUtility::Equal(entity.name, STEPBSplineCurve::EntityName)) {
			return ESTEPBSplineCurve;
		} else if (StringUtility::Equal(entity.name, STEPBSplineSurface::EntityName)) {
			return ESTEPBSplineSurface;
		}
	}

	return ESTEPEntityType::ESTEPNone;
}
Vector<STEPString::Entity> STEPString::CreateMultiEntity(const String& str)
{
	if (str.empty()) return Vector<Entity>();
	
	Vector<Entity> ret;
	int blacket = 0;
	Entity entity;
	for (size_t i = 1; i < str.size() - 2; i++) {
		if (str[i] == ' ') continue;
		if (str[i] == '(') {
			blacket++;
		}

		if (blacket == 0) {
			entity.name.push_back(str[i]);
		} else {
			entity.value.push_back(str[i]);
		}
		
		if (str[i] == ')') {
			blacket--;
			if (blacket == 0) {
				ret.push_back(entity);
				entity = Entity();
			}
		}
	}

	return ret;
}


Vector<String> STEPString::SplitValue(const String& value)
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

String STEPString::CreateRemoveLabelStr() const
{
	auto valuePoint = StringUtility::After(entity.value, ',');
	return StringUtility::RemoveLast(valuePoint, 2);
}

String STEPString::RemoveBracket(const String& str)
{
	auto valuePoint = StringUtility::Remove(str, '(');
	valuePoint = StringUtility::Remove(valuePoint, ')');
	return valuePoint;
}

bool STEPString::ValueToRef(const String& str, int& value)
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

bool STEPString::ValueToInt(const String& str, int& value)
{
	value = StringUtility::ToInt(str);
	return true;
}


 bool STEPString::ValueToFloat(const String& str, float& value)
{
	value = StringUtility::ToFloat(str);
	return true;
}

bool STEPString::IsAsterisk(const String& str)
{
	return StringUtility::Contains(str, "*");
}

// 文字がドル記号か判定
bool STEPString::IsDollar(const String& str)
{
	return StringUtility::Contains(str, "$");
}

bool STEPString::ValueToBool(const String& str, bool& value)
{
	if (StringUtility::Contains(str, ".T.")) { value = true;  return true; }
	if (StringUtility::Contains(str, ".F.")) { value = false;  return true; }
	assert(0); return false;
}

bool STEPString::ValueToLogical(const String& str, STEPLogicalType& value)
{
	if (StringUtility::Contains(str, ".T.")) { value = STEPLogicalType::True;  return true; }
	if (StringUtility::Contains(str, ".F.")) { value = STEPLogicalType::False;  return true; }
	if (StringUtility::Contains(str, ".U.")) { value = STEPLogicalType::UNDEFINED;  return true; }
	assert(0); return false;
}

bool STEPString::ValueToBSplineCurveForm(const String& str, STEPBSplineCurveFormType& value)
{
	if (StringUtility::Contains(str, ".POLYLINE_FORM.")) { value = STEPBSplineCurveFormType::POLYLINE;  return true; }
	if (StringUtility::Contains(str, ".CIRCULAR_ARC.")) { value = STEPBSplineCurveFormType::CIRCULAR_ARC;  return true; }
	if (StringUtility::Contains(str, ".ELLIPTIC_ARC.")) { value = STEPBSplineCurveFormType::ELLIPTIC_ARC;  return true; }
	if (StringUtility::Contains(str, ".PARABOLIC_ARC.")) { value = STEPBSplineCurveFormType::PARABOLIC_ARC;  return true; }
	if (StringUtility::Contains(str, ".HYPERBOLIC_ARC.")) { value = STEPBSplineCurveFormType::HYPERBOLIC_ARC;  return true; }
	if (StringUtility::Contains(str, ".UNSPECIFIED.")) { value = STEPBSplineCurveFormType::UNSPECIFIED;  return true; }
	assert(0); return false;
}

bool STEPString::ValueToKnot(const String& str, STEPKnotType& value)
{
	if (StringUtility::Contains(str, ".UNIFORM_KNOTS.")) { value = STEPKnotType::UNIFORM_KNOTS;  return true; }
	if (StringUtility::Contains(str, ".QUASI_UNIFORM_KNOTS.")) { value = STEPKnotType::QUASI_UNIFORM_KNOTS;  return true; }
	if (StringUtility::Contains(str, ".PIECEWISE_BEZIER_KNOTS.")) { value = STEPKnotType::PIECEWISE_BEZIER_KNOTS;  return true; }
	if (StringUtility::Contains(str, ".UNSPECIFIED.")) { value = STEPKnotType::UNSPECIFIED;  return true; }
	assert(0); return false;
}

bool STEPString::ValueToBSplineSurfaceForm(const String& str, STEPBSplineSurfaceFormType& value)
{
	if (StringUtility::Contains(str, ".PLANE_SURF.")) { value = STEPBSplineSurfaceFormType::PLANE_SURF;  return true; }
	if (StringUtility::Contains(str, ".CYLINDRICAL_SURF.")) { value = STEPBSplineSurfaceFormType::CYLINDRICAL_SURF;  return true; }
	if (StringUtility::Contains(str, ".CONICAL_SURF.")) { value = STEPBSplineSurfaceFormType::CONICAL_SURF;  return true; }
	if (StringUtility::Contains(str, ".SPHERICAL_SURF.")) { value = STEPBSplineSurfaceFormType::SPHERICAL_SURF;  return true; }
	if (StringUtility::Contains(str, ".TOROIDAL_SURF.")) { value = STEPBSplineSurfaceFormType::TOROIDAL_SURF;  return true; }
	if (StringUtility::Contains(str, ".SURF_OF_REVOLUTION.")) { value = STEPBSplineSurfaceFormType::SURF_OF_REVOLUTION;  return true; }
	if (StringUtility::Contains(str, ".RULED_SURF.")) { value = STEPBSplineSurfaceFormType::RULED_SURF;  return true; }
	if (StringUtility::Contains(str, ".GENERALISED_CONE.")) { value = STEPBSplineSurfaceFormType::GENERALISED_CONE;  return true; }
	if (StringUtility::Contains(str, ".QUADRIC_SURF.")) { value = STEPBSplineSurfaceFormType::QUADRIC_SURF;  return true; }
	if (StringUtility::Contains(str, ".SURF_OF_LINEAR_EXTRUSION.")) { value = STEPBSplineSurfaceFormType::SURF_OF_LINEAR_EXTRUSION;  return true; }
	if (StringUtility::Contains(str, ".UNSPECIFIED.")) { value = STEPBSplineSurfaceFormType::UNSPECIFIED;  return true; }
	assert(0); return false;
}


String STEPString::ToString() const
{
	return
		"#" + StringUtility::ToString(id) +
		"=" + entity.name + entity.value;
}

STEPStruct::~STEPStruct()
{
	for (auto& v : points) { delete v.second; }
	for (auto& v : lines) { delete v.second; }
	for (auto& v : circles) { delete v.second; }
	for (auto& v : cylindricalSurface) { delete v.second; }
	for (auto& v : conicalSurface) { delete v.second; }
	for (auto& v : toroidalSurface) { delete v.second; }
	for (auto& v : planes) { delete v.second; }
	for (auto& v : vectors) { delete v.second; }
	for (auto& v : directions) { delete v.second; }
	for (auto& v : edgeCurve) { delete v.second; }
	for (auto& v : axis2Placement3D) { delete v.second; }
	for (auto& v : vertexPoint) { delete v.second; }
	for (auto& v : interSectionCurve) { delete v.second; }
	for (auto& v : edgeLoop) { delete v.second; }
	for (auto& v : polyLoop) { delete v.second; }
	for (auto& v : faceOuterBound) { delete v.second; }
	for (auto& v : faceBound) { delete v.second; }
	for (auto& v : quasiUniformCurve) { delete v.second; }
	for (auto& v : orientedEdge) { delete v.second; }
	for (auto& v : advancedFace) { delete v.second; }
	for (auto& v : faceSurface) { delete v.second; }
	for (auto& v : closedShell) { delete v.second; }
	for (auto& v : openShell) { delete v.second; }
	for (auto& v : bSplineCurve) { delete v.second; }
	for (auto& v : bSplineSurfaceWithKnots) { delete v.second; }
	for (auto& v : bSplineSurface) { delete v.second; }

}

void STEPEntityBase::NotDefineEntity(const String& str)
{

	"LENGTH_MEASURE_WITH_UNIT";
	"ADVANCED_BREP_SHAPE_REPRESENTATION";
	"MANIFOLD_SOLID_BREP";
	"UNCERTAINTY_MEASURE_WITH_UNIT";
	"SHAPE_DEFINITION_REPRESENTATION";
	"SHAPE_REPRESENTATION_RELATIONSHIP";

	auto stepStr = STEPString::Create(str);
	for (const auto& key : g_ignoreEqualEntity) {
		if (StringUtility::Equal(stepStr.entity.name, key)) {
			return;
		}
	}

	for (const auto& key : g_ignoreContainsEntity) {
		if (StringUtility::Contains(str, key)) {
			return;
		}
	}

	// 検討していないエンティティ
	printf("%s\n", str.data());
}


void STEPEntityBase::Fetch(STEPEntityBase* data, const STEPString& stepStr)
{
	data->id = stepStr.id;
	data->str = stepStr.ToString();
}
bool STEPEntityBase::ShowBranch(STEPUIContext& ui, bool select)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	if (select) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	auto open = ImGui::TreeNodeEx(str.data(), flags);
	if (ImGui::IsItemClicked()) {
		ui.pSelect = this;
		if (GetType() == ESTEPAdvancedFace) {
			ui.pNode->AddDebugNode(ui, this);
		}
	}

	if (ImGui::BeginPopupContextItem()) {
		if (GetType() == ESTEPAdvancedFace) {
			if (ImGui::MenuItem("OutputFile")) {
				auto str = Dump(DebugOption());
				FileWriter writer;
				String shell = "#0=CLOSED_SHELL('',(#" + StringUtility::ToString(id) + "));\n";
				writer.Open(FileUtility::RemoveExtension(ui.filePath) + "_FACE" + StringUtility::ToString(id) + "_orig.step");
				writer.Write("DATA;\n");
				writer.Write(shell);
				writer.Write(str, true);
				writer.Close();
			}
		}

		if (ImGui::MenuItem("DebugPrintf")) {
			auto str = Dump(DebugOption());
			DebugPrintf::StringStr("------------------------------\n");
			DebugPrintf::StringStr(str);
			DebugPrintf::StringStr("------------------------------\n");
		}
		ImGui::EndPopup();
	}


	return open;
}

void STEPEntityBase::ShowLeaf(STEPUIContext& ui)
{
	ImGui::TreeNodeEx(str.data(),
		ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
}

void STEPEntityBase::PrintfRaw()
{
	DebugPrintf::StringStr(str); DebugPrintf::NewLine();
}

String STEPEntityBase::ToString() const
{
	return str + "\n";
}


void STEPPoint::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPPoint();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	values = STEPString::SplitValue(values[1]);
	if (!STEPString::ValueToFloat(values[0], data->pos.x)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[1], data->pos.y)) { assert(0); return; }
	if (values.size() == 3) {
		if (!STEPString::ValueToFloat(values[2], data->pos.z)) { assert(0); return; }
	}
	step.points[data->id] = data;
}

void STEPPoint::FetchData(const STEPStruct& step) {}

String STEPPoint::Dump(const DebugOption& option) { return STEPEntityBase::ToString(); }
void STEPPoint::ShowUI(STEPUIContext& ui) { ShowLeaf(ui); }

void STEPDirection::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPDirection();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	values = STEPString::SplitValue(values[1]);
	if (!STEPString::ValueToFloat(values[0], data->direction.x)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[1], data->direction.y)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[2], data->direction.z)) { assert(0); return; }
	step.directions[data->id] = data;
}

void STEPDirection::FetchData(const STEPStruct& step) {}
String STEPDirection::Dump(const DebugOption& option) { return STEPEntityBase::ToString(); }
void STEPDirection::ShowUI(STEPUIContext& ui) { ShowLeaf(ui); }

void STEPVector::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPVector();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->direction.first)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[2], data->length)) { assert(0); return; }

	step.vectors[data->id] = data;
}

void STEPVector::FetchData(const STEPStruct& step)
{
	direction.second = FindSetData2(step, step.directions, direction.first);
	vector = glm::normalize(direction.second->direction) * length;
}

String STEPVector::Dump(const DebugOption& option)
{
	String str = ToString();
	if (direction.second) { str += direction.second->Dump(option); }
	return str;
}

void STEPVector::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (direction.second) { direction.second->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPLine::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPLine();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->point.first)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[2], data->vector.first)) { assert(0); return; }
	step.lines[data->id] = data;
}

void STEPLine::FetchData(const STEPStruct& step)
{
	point.second = FindSetData2(step, step.points, point.first);
	vector.second = FindSetData2(step, step.vectors, vector.first);
}

Polyline STEPLine::CreatePolyline()
{
	return Polyline(Vector<Vector3>{point.second->pos, point.second->pos + vector.second->vector});
}

String STEPLine::Dump(const DebugOption& option)
{
	String str = ToString();
	if (point.second) { str += point.second->Dump(option); }
	if (vector.second) { str += vector.second->Dump(option); }
	return str;
}


void STEPLine::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (point.second) { point.second->ShowUI(ui); }
		if (vector.second) { vector.second->ShowUI(ui); }
		ImGui::TreePop();
	}
}


void STEPAxis2Placement3D::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPAxis2Placement3D();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->raw.pointRef)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[2], data->raw.dirRef1)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[3], data->raw.dirRef2)) { assert(0); return; }

	step.axis2Placement3D[data->id] = data;
}

void STEPAxis2Placement3D::FetchData(const STEPStruct& step)
{
	data.point0 = FindSetData2(step, step.points, raw.pointRef);
	data.point = data.point0->pos;

	data.direction1 = FindSetData2(step, step.directions, raw.dirRef1);
	if (data.direction1) data.dir1 = data.direction1->direction;


	data.direction2 = FindSetData2(step, step.directions, raw.dirRef2);
	if (data.direction2) data.dir2 = data.direction2->direction;
}

String STEPAxis2Placement3D::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (data.point0) { str += data.point0->Dump(option); }
	if (data.direction1) { str += data.direction1->Dump(option); }
	if (data.direction2) { str += data.direction2->Dump(option); }
	return str;
}

void STEPAxis2Placement3D::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.point0) { data.point0->ShowUI(ui); }
		if (data.direction1) { data.direction1->ShowUI(ui); }
		if (data.direction2) { data.direction2->ShowUI(ui); }
		ImGui::TreePop();
	}
}


Vector3 STEPCircle::GetPoint(const Vector3& begin, const Vector3& end, float parameter) const
{
	return Circle::GetPoint(rad, glm::normalize(axis.second->data.U()), glm::normalize(axis.second->data.V()), 
		axis.second->data.point, begin, end, parameter);
}

Polyline STEPCircle::CreatePolyline(const Vector3& begin, const Vector3& end) const
{
	if (MathHelper::IsSame(begin, end)) {
		return Circle::CreateLine(rad, CIRCLE_SUBDIVISION_NUM,
			glm::normalize(axis.second->data.U()), glm::normalize(axis.second->data.V()), axis.second->data.point);
	} else {
		return Circle::CreateArc(rad, CIRCLE_SUBDIVISION_NUM,
			glm::normalize(axis.second->data.U()), glm::normalize(axis.second->data.V()), axis.second->data.point, begin, end);
	}
}

void STEPCircle::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPCircle();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->axis.first)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[2], data->rad)) { assert(0); return; }
	step.circles[data->id] = data;
}

void STEPCircle::FetchData(const STEPStruct& step)
{
	axis.second = FindSetData2(step, step.axis2Placement3D, axis.first);
}

String STEPCircle::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (axis.second) { str += axis.second->Dump(option); }
	return str;
}

void STEPCircle::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (axis.second) { axis.second->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPCylindricalSurface::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPCylindricalSurface();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->axis.first)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[2], data->rad)) { assert(0); return; }
	step.cylindricalSurface[data->id] = data;
}

void STEPCylindricalSurface::FetchData(const STEPStruct& step)
{
	axis.second = FindSetData2(step, step.axis2Placement3D, axis.first);
}

String STEPCylindricalSurface::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (axis.second) { str += axis.second->Dump(option); }
	return str;
}

void STEPCylindricalSurface::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (axis.second) { axis.second->ShowUI(ui); }
		ImGui::TreePop();
	}
}


void STEPToroidalSurface::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPToroidalSurface();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->axis.first)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[2], data->majorRadius)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[3], data->minorRadius)) { assert(0); return; }
	step.toroidalSurface[data->id] = data;
}

void STEPToroidalSurface::FetchData(const STEPStruct& step)
{
	axis.second = FindSetData2(step, step.axis2Placement3D, axis.first);
}

String STEPToroidalSurface::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (axis.second) { str += axis.second->Dump(option); }
	return str;
}

void STEPToroidalSurface::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (axis.second) { axis.second->ShowUI(ui); }
		ImGui::TreePop();
	}
}


void STEPConicalSurface::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPConicalSurface();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->axis.first)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[2], data->radius)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[3], data->angle)) { assert(0); return; }
	step.conicalSurface[data->id] = data;
}

void STEPConicalSurface::FetchData(const STEPStruct& step)
{
	axis.second = FindSetData2(step, step.axis2Placement3D, axis.first);
}

String STEPConicalSurface::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (axis.second) { str += axis.second->Dump(option); }
	return str;
}

void STEPConicalSurface::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (axis.second) { axis.second->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPPlane::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPPlane();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->axis.first)) { assert(0); return; }
	step.planes[data->id] = data;
}

void STEPPlane::FetchData(const STEPStruct& step)
{
	axis.second = FindSetData2(step, step.axis2Placement3D, axis.first);
}

String STEPPlane::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (axis.second) { str += axis.second->Dump(option); }
	return str;
}

void STEPPlane::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (axis.second) { axis.second->ShowUI(ui); }
		ImGui::TreePop();
	}
}

Polyline STEPInterSectionCurve::Data::CreatePolyline(const Vector3& begin, const Vector3& end) const
{
	return KI::STEP::Intersection::CreatePolyline(*this, begin, end);
}

void STEPInterSectionCurve::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPInterSectionCurve();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->raw.curveId)) { assert(0); return; }
	auto split = STEPString::SplitValue(values[2]);
	if (!STEPString::ValueToRef(split[0], data->raw.geomId0)) { assert(0); return; }
	if (!STEPString::ValueToRef(split[1], data->raw.geomId1)) { assert(0); return; }
	if (StringUtility::Equal(values[3], ".CURVE_3D.")) { data->data.type = Data::Type::Curve3D; }
	step.interSectionCurve[data->id] = data;
}

void STEPInterSectionCurve::FetchData(const STEPStruct& step)
{
	data.curve0.pLine = FindSetData2(step, step.lines, raw.curveId);
	if (!data.curve0.IsActive()) {
		data.curve0.pCircle = FindSetData2(step, step.circles, raw.curveId);
	}
	data.surf0.pCylinderSurface = FindSetData2(step, step.cylindricalSurface, raw.geomId0);
	if (!data.surf0.IsActive()) {
		data.surf0.pPlane = FindSetData2(step, step.planes, raw.geomId0);
	}

	data.surf1.pCylinderSurface = FindSetData2(step, step.cylindricalSurface, raw.geomId1);
	if (!data.surf1.IsActive()) {
		data.surf1.pPlane = FindSetData2(step, step.planes, raw.geomId1);
	}

	if (!data.curve0.IsActive() ||
		!data.surf0.IsActive() ||
		!data.surf1.IsActive()) {
		assert(0);
	}
}


String STEPInterSectionCurve::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (data.curve0.pLine) { str += data.curve0.pLine->Dump(option); }
	if (data.curve0.pCircle) { str += data.curve0.pCircle->Dump(option); }
	if (data.surf0.pPlane) { str += data.surf0.pPlane->Dump(option); }
	if (data.surf1.pCylinderSurface) { str += data.surf1.pCylinderSurface->Dump(option); }
	return str;
}

void STEPInterSectionCurve::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.curve0.pLine) { data.curve0.pLine->ShowUI(ui); }
		if (data.curve0.pCircle) { data.curve0.pCircle->ShowUI(ui); }
		if (data.surf0.pPlane) { data.surf0.pPlane->ShowUI(ui); }
		if (data.surf0.pCylinderSurface) { data.surf0.pCylinderSurface->ShowUI(ui); }
		if (data.surf1.pPlane) { data.surf1.pPlane->ShowUI(ui); }
		if (data.surf1.pCylinderSurface) { data.surf1.pCylinderSurface->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPVertexPoint::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPVertexPoint();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->point.first)) { assert(0); return; }
	step.vertexPoint[data->id] = data;
}

void STEPVertexPoint::FetchData(const STEPStruct& step)
{
	point.second = FindSetData2(step, step.points, point.first);
}

String STEPVertexPoint::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (point.second) { str += point.second->Dump(option); }
	return str;
}

void STEPVertexPoint::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (point.second) { point.second->ShowUI(ui); }
		ImGui::TreePop();
	}
}

Vector3 STEPEdgeCurve::GetPoint(float parameter)
{
	if (data.line) {
		auto len = data.GetEnd() - data.GetBegin();
		return data.GetBegin() + len * parameter;
	}

	if (data.circle) {
		return data.circle->GetPoint(data.GetBegin(), data.GetEnd(), parameter);
	}

	if (data.intersectionCurve) {
		assert(0);
		return Vector3();
	}

	return Vector3();
}
Polyline STEPEdgeCurve::CreatePolyline() const
{
	if (data.line) {
		return Polyline(Vector<Vector3>{ data.GetBegin(), data.GetEnd() });
	}

	if (data.circle) {
		return data.circle->CreatePolyline(data.GetBegin(), data.GetEnd());
	}

	if (data.intersectionCurve) {
		return data.intersectionCurve->data.CreatePolyline(data.GetBegin(), data.GetEnd());
	}

	if (data.quasiUniformCurve) {
		return data.quasiUniformCurve->CreatePolyline(data.GetBegin(), data.GetEnd());
	}

	if (data.bsplineCurve) {
		return data.bsplineCurve->CreatePolyline(data.GetBegin(), data.GetEnd());
	}
	assert(0);
	return Polyline();
}

void STEPEdgeCurve::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPEdgeCurve();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->raw.vertRef0)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[2], data->raw.vertRef1)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[3], data->raw.lineRef2)) { assert(0); return; }
	if (!STEPString::ValueToBool(values[4], data->raw.sameSense)) { assert(0); return; }

	step.edgeCurve[data->id] = data;
}

void STEPEdgeCurve::FetchData(const STEPStruct& step)
{
	data.pPoint0 = FindSetData2(step, step.vertexPoint, raw.vertRef0);
	data.begin = data.pPoint0->point.second->pos;

	data.pPoint1 = FindSetData2(step, step.vertexPoint, raw.vertRef1);
	data.end = data.pPoint1->point.second->pos;

	data.line = FindSetData2(step, step.lines, raw.lineRef2);
	if (!data.line) {
		data.circle = FindSetData2(step, step.circles, raw.lineRef2);
	}
	if (!data.line && !data.circle) {
		data.intersectionCurve = FindSetData2(step, step.interSectionCurve, raw.lineRef2);
	}

	if (!data.line && !data.circle && !data.intersectionCurve) {
		data.quasiUniformCurve = FindSetData2(step, step.quasiUniformCurve, raw.lineRef2);
	}

	if (!data.line && !data.circle && !data.intersectionCurve && !data.quasiUniformCurve) {
		data.bsplineCurve = FindSetData2(step, step.bSplineCurve, raw.lineRef2);
	}

	data.sameSense = raw.sameSense;
}

String STEPEdgeCurve::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (data.pPoint0) { str += data.pPoint0->Dump(option); }
	if (data.pPoint1) { str += data.pPoint1->Dump(option); }
	if (data.line) { str += data.line->Dump(option); }
	if (data.circle) { str += data.circle->Dump(option); }
	if (data.intersectionCurve) { str += data.intersectionCurve->Dump(option); }
	if (data.quasiUniformCurve) { str += data.quasiUniformCurve->Dump(option); }
	if (data.bsplineCurve) { str += data.bsplineCurve->Dump(option); }
	return str;
}


void STEPEdgeCurve::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.pPoint0) { data.pPoint0->ShowUI(ui); }
		if (data.pPoint1) { data.pPoint1->ShowUI(ui); }
		if (data.line) { data.line->ShowUI(ui); }
		if (data.circle) { data.circle->ShowUI(ui); }
		if (data.intersectionCurve) { data.intersectionCurve->ShowUI(ui); }
		if (data.quasiUniformCurve) { data.quasiUniformCurve->ShowUI(ui); }
		if (data.bsplineCurve) { data.bsplineCurve->ShowUI(ui); }
		ImGui::TreePop();
	}
}



Polyline STEPOrientedEdge::Data::CreatePolyline() const
{
	auto polyline = edgeCurve->CreatePolyline();
	if (!orient) {
		polyline.Reverse();
	}
	return polyline;
}

void STEPOrientedEdge::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPOrientedEdge();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToRef(values[1], data->raw.vertRef0)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[2], data->raw.vertRef1)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[3], data->raw.edgeCurveRef2)) { assert(0); return; }
	if (!STEPString::ValueToBool(values[4], data->raw.orient)) { assert(0); return; }
	step.orientedEdge[data->id] = data;
}


void STEPOrientedEdge::FetchData(const STEPStruct& step)
{
	data.edgeCurve = FindSetData2(step, step.edgeCurve, raw.edgeCurveRef2);
	if (raw.vertRef0 == STEPEnum::ASTERISK) {
		data.begin = data.edgeCurve->data.begin;
	} else {
		data.vertex0 = FindSetData2(step, step.vertexPoint, raw.vertRef0);
		data.begin = data.vertex0->point.second->pos;
	}

	if (raw.vertRef1 == STEPEnum::ASTERISK) {
		data.end = data.edgeCurve->data.end;
	} else {
		data.vertex1 = FindSetData2(step, step.vertexPoint, raw.vertRef1);
		data.end = data.vertex1->point.second->pos;
	}

	data.orient = raw.orient;
}

String STEPOrientedEdge::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (data.edgeCurve) { str += data.edgeCurve->Dump(option); }
	if (data.vertex0) { str += data.vertex0->Dump(option); }
	if (data.vertex1) { str += data.vertex1->Dump(option); }
	return str;
}

void STEPOrientedEdge::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.edgeCurve) { data.edgeCurve->ShowUI(ui); }
		if (data.vertex0) { data.vertex0->ShowUI(ui); }
		if (data.vertex1) { data.vertex1->ShowUI(ui); }
		ImGui::TreePop();
	}
}

PolylineList STEPPolyLoop::Data::CreatePolyline(int id) const
{
	Vector<Vector3> lines(points.size() * 2);
	for (int i = 0; i < points.size(); i++) {
		lines[2 * i] = points[i]->pos;
		lines[2 * i + 1] = points[(i + 1) % points.size()]->pos;
	}
	return PolylineList(id, Polyline(std::move(lines), Polyline::DrawType::Lines));
}


void STEPPolyLoop::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPPolyLoop();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	values = STEPString::SplitValue(values[1]);

	data->raw.idRef.resize(values.size());
	for (int i = 0; i < data->raw.idRef.size(); i++) {
		if (!STEPString::ValueToRef(values[i], data->raw.idRef[i])) { assert(0); return; }
	}
	step.polyLoop[data->id] = data;
}

void STEPPolyLoop::FetchData(const STEPStruct& step)
{
	data.points.resize(raw.idRef.size());
	for (auto i = 0; i < raw.idRef.size(); i++) {
		data.points[i] = FindSetData2(step, step.points, raw.idRef[i]);
	}
}

String STEPPolyLoop::Dump(const DebugOption& option)
{
	auto str = ToString();
	for (size_t i = 0; i < data.points.size(); i++) {
		str += data.points[i]->Dump(option);
	}
	return str;
}

void STEPPolyLoop::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		for (size_t i = 0; i < data.points.size(); i++) {
			data.points[i]->ShowUI(ui);
		}

		ImGui::TreePop();
	}
}


PolylineList STEPEdgeLoop::Data::CreatePolyline() const
{
	Vector<Vector3> points;
	PolylineList polyline;
	for (size_t i = 0; i < orientedEdges.size(); ++i) {
		if (orientedEdges[i] == nullptr) { continue; }
		if (orientedEdges[i]->data.IsLine())
		//if (false) 
		{
			Polyline line;
			line.Add(orientedEdges[i]->data.GetBegin());
			line.Add(orientedEdges[i]->data.GetEnd());
			polyline.Add(orientedEdges[i]->id, std::move(line));
		} else {
			polyline.Add(orientedEdges[i]->id, orientedEdges[i]->data.CreatePolyline());
		}
	}

	return polyline;
}


void STEPEdgeLoop::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPEdgeLoop();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	values = STEPString::SplitValue(values[1]);

	data->raw.idRef.resize(values.size());
	for (int i = 0; i < data->raw.idRef.size(); i++) {
		if (!STEPString::ValueToRef(values[i], data->raw.idRef[i])) { assert(0); return; }
	}
	step.edgeLoop[data->id] = data;
}

void STEPEdgeLoop::FetchData(const STEPStruct& step)
{
	data.orientedEdges.resize(raw.idRef.size());
	for (auto i = 0; i < raw.idRef.size(); i++) {
		data.orientedEdges[i] = FindSetData2(step, step.orientedEdge, raw.idRef[i]);
	}
}

String STEPEdgeLoop::Dump(const DebugOption& option)
{
	auto str = ToString();
	for (auto orientedEdge : data.orientedEdges) {
		if (orientedEdge == nullptr) { continue; }
		str += orientedEdge->Dump(option);
	}
	return str;
}

void STEPEdgeLoop::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		for (auto orientedEdge : data.orientedEdges) {
			if (orientedEdge == nullptr) { continue; }
			orientedEdge->ShowUI(ui);
		}
		ImGui::TreePop();
	}
}



PolylineList STEPFaceBoundBase::Data::CreatePolyline() const
{
	if (edgeLoop) { return edgeLoop->data.CreatePolyline(); }
	if (polyLoop) { return polyLoop->data.CreatePolyline(polyLoop->id); }

	return PolylineList();
}


void STEPFaceBoundBase::Fetch(STEPStruct& step, const STEPString& stepStr, STEPFaceBoundBase* data)
{
	auto values = STEPString::SplitValue(stepStr.entity.value);
	STEPEntityBase::Fetch(data, stepStr);
	if (!STEPString::ValueToRef(values[1], data->raw.idRef0)) { assert(0); return; }
	if (!STEPString::ValueToBool(values[2], data->raw.orient)) { assert(0); return; }
}

void STEPFaceBoundBase::FetchData(const STEPStruct& step)
{
	data.edgeLoop = FindSetData2(step, step.edgeLoop, raw.idRef0);
	if (!data.edgeLoop) {
		data.polyLoop = FindSetData2(step, step.polyLoop, raw.idRef0);
	}

	data.orient = raw.orient;
}

String STEPFaceBoundBase::Dump(const DebugOption& option)
{
	auto str = ToString();
	if (data.edgeLoop) { str += data.edgeLoop->Dump(option); }
	if (data.polyLoop) { str += data.polyLoop->Dump(option); }
	return str;
}

void STEPFaceBoundBase::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.edgeLoop) { data.edgeLoop->ShowUI(ui); }
		if (data.polyLoop) { data.polyLoop->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPFaceOuterBound::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPFaceOuterBound();
	STEPFaceBoundBase::Fetch(step, stepStr, data);
	step.faceOuterBound[data->id] = data;
}

void STEPFaceBound::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPFaceBound();
	STEPFaceBoundBase::Fetch(step, stepStr, data);
	step.faceBound[data->id] = data;
}

STEPFaceBase::Data::CylidnerEdge STEPFaceBase::Data::SearchConicalEdge(const STEPConicalSurface* pConical) const
{
	CylidnerEdge ret;
	bool setBegin = false; bool setEnd = false;
	for (const auto& face : faceBound) {
		auto edgeLoop = face->data.edgeLoop;
		if (!edgeLoop) { continue; }
		for (const auto& edge : edgeLoop->data.orientedEdges) {
			auto begin = edge->data.GetBegin() - pConical->axis.second->data.point;
			auto end = edge->data.GetEnd() - pConical->axis.second->data.point;
			auto v0 = glm::dot(begin, pConical->axis.second->data.Normal());
			auto v1 = glm::dot(end, pConical->axis.second->data.Normal());

			auto sameBeginDir = MathHelper::IsSameDir(edge->data.GetBegin() - edge->data.GetEnd(), pConical->axis.second->data.Normal());
			auto sameEndDir = MathHelper::IsSameDir(edge->data.GetEnd() - edge->data.GetBegin(), pConical->axis.second->data.Normal());
			{
				if (ret.maxZ.first < v0) { ret.maxZ.first = v0; ret.maxZ.second = edge->data.GetBegin(); }
				if (ret.maxZ.first < v1) { ret.maxZ.first = v1; ret.maxZ.second = edge->data.GetEnd(); }

				if (ret.minZ.first > v0) { ret.minZ.first = v0; ret.minZ.second = edge->data.GetBegin(); }
				if (ret.minZ.first > v1) { ret.minZ.first = v1; ret.minZ.second = edge->data.GetEnd(); }

				if (sameBeginDir) {
					ret.begin = edge->data.GetBegin();
					setBegin = true;
				}

				if (sameEndDir) {
					ret.end = edge->data.GetEnd();
					setEnd = true;
				}
			}
		}
	}

	for (const auto& face : faceOuterBound) {
		auto edgeLoop = face->data.edgeLoop;
		if (!edgeLoop) { continue; }
		for (const auto& edge : edgeLoop->data.orientedEdges) {
			auto begin = edge->data.GetBegin() - pConical->axis.second->data.point;
			auto end = edge->data.GetEnd() - pConical->axis.second->data.point;
			auto v0 = glm::dot(begin, pConical->axis.second->data.Normal());
			auto v1 = glm::dot(end, pConical->axis.second->data.Normal());

			auto sameBeginDir = MathHelper::IsSameDir(edge->data.GetBegin() - edge->data.GetEnd(), pConical->axis.second->data.Normal());
			auto sameEndDir = MathHelper::IsSameDir(edge->data.GetEnd() - edge->data.GetBegin(), pConical->axis.second->data.Normal());
			{
				if (ret.maxZ.first < v0) { ret.maxZ.first = v0; ret.maxZ.second = edge->data.GetBegin(); }
				if (ret.maxZ.first < v1) { ret.maxZ.first = v1; ret.maxZ.second = edge->data.GetEnd(); }

				if (ret.minZ.first > v0) { ret.minZ.first = v0; ret.minZ.second = edge->data.GetBegin(); }
				if (ret.minZ.first > v1) { ret.minZ.first = v1; ret.minZ.second = edge->data.GetEnd(); }

				if (sameBeginDir) {
					ret.begin = edge->data.GetBegin();
					setBegin = true;
				}

				if (sameEndDir) {
					ret.end = edge->data.GetEnd();
					setEnd = true;
				}
			}
		}
	}

	// 母線1本の時
	if (setBegin && !setEnd) { ret.end = ret.begin; }
	if (!setBegin && setEnd) { ret.begin = ret.end; }
	// 母線がないとき
	if (!setBegin && !setEnd) {
		ret.begin = pConical->axis.second->data.point + pConical->axis.second->data.U() * pConical->radius;
		ret.end = ret.begin;
	}
	return ret;
}

STEPFaceBase::Data::ToroidalEdge STEPFaceBase::Data::SearchToroidalEdge(const STEPToroidalSurface* pToroidal) const
{
	auto center = pToroidal->axis.second->data.point;
	auto xAxis = glm::normalize(pToroidal->axis.second->data.U());
	auto yAxis = glm::normalize(pToroidal->axis.second->data.V());
	auto zAxis = glm::normalize(pToroidal->axis.second->data.Normal());
	ToroidalEdge ret;
	for (const auto& face : faceBound) {
		auto edgeLoop = face->data.edgeLoop;
		if (!edgeLoop) { continue; }
		for (const auto& edge : edgeLoop->data.orientedEdges) {
			auto begin = Torus::ToUV(center, xAxis, yAxis, zAxis, pToroidal->majorRadius, edge->data.GetBegin());
			auto mid3D = edge->data.edgeCurve->GetPoint(0.5f);
			auto mid = Torus::ToUV(center, xAxis, yAxis, zAxis, pToroidal->majorRadius, mid3D);
			auto end = Torus::ToUV(center, xAxis, yAxis, zAxis, pToroidal->majorRadius, edge->data.GetEnd());
			auto diff1 = mid - begin; auto diff2 = end - mid;
			
			auto beginAngle = MathHelper::ToRadian(edge->data.GetBegin() - center, xAxis, yAxis, 1);
			auto midAngle = MathHelper::ToRadian(mid3D - center, xAxis, yAxis, 1);
			auto endAngle = MathHelper::ToRadian(edge->data.GetEnd() - center, xAxis, yAxis, 1);
			auto v1 = MathHelper::UnWrapDiffRad(endAngle, midAngle);
			auto v2 = MathHelper::UnWrapDiffRad(midAngle, beginAngle);

			if (MathHelper::IsSameRad(diff1.x, 0.0f) && MathHelper::IsSameRad(diff2.x, 0.0f)) {
				// v 方向
				ret.vDir = (v2 - v1) > 0.0f;
			} else if(MathHelper::IsSameRad(diff1.y, 0.0f) && MathHelper::IsSameRad(diff2.y, 0.0f)){
				// u 方向
				ret.uDir = (v2 - v1) > 0.0f;
			} else {
				assert(0);
			}

			begin.x = MathHelper::Normalize0_PI2(begin.x);
			begin.y = MathHelper::Normalize0_PI2(begin.y);
			ret.uBegin = std::min(begin.x, ret.uBegin);
			ret.uEnd = std::max(begin.x, ret.uEnd);
			ret.vBegin = std::min(begin.y, ret.vBegin);
			ret.vEnd = std::max(begin.y, ret.vEnd);

			end.x = MathHelper::Normalize0_PI2(end.x);
			end.y = MathHelper::Normalize0_PI2(end.y);

			ret.uBegin = std::min(end.x, ret.uBegin);
			ret.uEnd = std::max(end.x, ret.uEnd);
			ret.vBegin = std::min(end.y, ret.vBegin);
			ret.vEnd = std::max(end.y, ret.vEnd);
		}
	}


	for (const auto& face : faceOuterBound) {
		auto edgeLoop = face->data.edgeLoop;
		if (!edgeLoop) { continue; }
		for (const auto& edge : edgeLoop->data.orientedEdges) {
			auto begin = Torus::ToUV(center, xAxis, yAxis, zAxis, pToroidal->majorRadius, edge->data.GetBegin());
			begin.x = MathHelper::Normalize0_PI2(begin.x);
			begin.y = MathHelper::Normalize0_PI2(begin.y);
			ret.uBegin = std::min(begin.x, ret.uBegin);
			ret.uEnd = std::max(begin.x, ret.uEnd);
			ret.vBegin = std::min(begin.y, ret.vBegin);
			ret.vEnd = std::max(begin.y, ret.vEnd);

			auto end = Torus::ToUV(center, xAxis, yAxis, zAxis, pToroidal->majorRadius, edge->data.GetEnd());
			end.x = MathHelper::Normalize0_PI2(end.x);
			end.y = MathHelper::Normalize0_PI2(end.y);
			ret.uBegin = std::min(end.x, ret.uBegin);
			ret.uEnd = std::max(end.x, ret.uEnd);
			ret.vBegin = std::min(end.y, ret.vBegin);
			ret.vEnd = std::max(end.y, ret.vEnd);
		}
	}

	return ret;
}
STEPFaceBase::Data::CylidnerEdge STEPFaceBase::Data::SearchCylinderEdge(const STEPCylindricalSurface* pCylinder) const
{
	CylidnerEdge ret;
	bool setBegin = false; bool setEnd = false;
	for (const auto& face : faceBound) {
		auto edgeLoop = face->data.edgeLoop;
		if (!edgeLoop) { continue; }
		for (const auto& edge : edgeLoop->data.orientedEdges) {
			auto begin = edge->data.GetBegin() - pCylinder->axis.second->data.point;
			auto end = edge->data.GetEnd() - pCylinder->axis.second->data.point;
			auto v0 = glm::dot(begin, pCylinder->axis.second->data.Normal());
			auto v1 = glm::dot(end, pCylinder->axis.second->data.Normal());

			auto sameBeginDir = MathHelper::IsSameDir(edge->data.GetBegin() - edge->data.GetEnd(), pCylinder->axis.second->data.Normal());
			auto sameEndDir = MathHelper::IsSameDir(edge->data.GetEnd() - edge->data.GetBegin(), pCylinder->axis.second->data.Normal());
			{
				if (ret.maxZ.first < v0) { ret.maxZ.first = v0; ret.maxZ.second = edge->data.GetBegin(); }
				if (ret.maxZ.first < v1) { ret.maxZ.first = v1; ret.maxZ.second = edge->data.GetEnd(); }

				if (ret.minZ.first > v0) { ret.minZ.first = v0; ret.minZ.second = edge->data.GetBegin(); }
				if (ret.minZ.first > v1) { ret.minZ.first = v1; ret.minZ.second = edge->data.GetEnd(); }

				if (sameBeginDir) {
					ret.begin = edge->data.GetBegin();
					setBegin = true;
				}

				if (sameEndDir) {
					ret.end = edge->data.GetEnd();
					setEnd = true;
				}
			} 
		}
	}

	for (const auto& face : faceOuterBound) {
		auto edgeLoop = face->data.edgeLoop;
		if (!edgeLoop) { continue; }
		for (const auto& edge : edgeLoop->data.orientedEdges) {
			auto begin = edge->data.GetBegin() - pCylinder->axis.second->data.point;
			auto end = edge->data.GetEnd() - pCylinder->axis.second->data.point;
			auto v0 = glm::dot(begin, pCylinder->axis.second->data.Normal());
			auto v1 = glm::dot(end, pCylinder->axis.second->data.Normal());

			auto sameBeginDir = MathHelper::IsSameDir(edge->data.GetBegin() - edge->data.GetEnd(), pCylinder->axis.second->data.Normal());
			auto sameEndDir = MathHelper::IsSameDir(edge->data.GetEnd() - edge->data.GetBegin(), pCylinder->axis.second->data.Normal());
			{
				if (ret.maxZ.first < v0) { ret.maxZ.first = v0; ret.maxZ.second = edge->data.GetBegin(); }
				if (ret.maxZ.first < v1) { ret.maxZ.first = v1; ret.maxZ.second = edge->data.GetEnd(); }

				if (ret.minZ.first > v0) { ret.minZ.first = v0; ret.minZ.second = edge->data.GetBegin(); }
				if (ret.minZ.first > v1) { ret.minZ.first = v1; ret.minZ.second = edge->data.GetEnd(); }

				if (sameBeginDir) {
					ret.begin = edge->data.GetBegin();
					setBegin = true;
				} 
				
				if(sameEndDir) {
					ret.end = edge->data.GetEnd();
					setEnd = true;
				}
			}
		}
	}

	// 母線1本の時
	if (setBegin && !setEnd) { ret.end = ret.begin; }
	if (!setBegin && setEnd) { ret.begin = ret.end; }
	// 母線がないとき
	if (!setBegin && !setEnd) {
		ret.begin = pCylinder->axis.second->data.point + pCylinder->axis.second->data.U() * pCylinder->rad;
		ret.end = ret.begin;
	}
	return ret;
}
PolylineList STEPFaceBase::Data::CreateBoundPolyline() const
{
	PolylineList bound;
	for (const auto& face : faceBound) { bound.Add(face->data.CreatePolyline()); }
	return bound;
}

PolylineList STEPFaceBase::Data::CreateOuterBoundPolyline() const
{
	PolylineList outerBound;
	for (const auto& face : faceOuterBound) { outerBound.Add(face->data.CreatePolyline()); }
	return outerBound;
}

Mesh STEPFaceBase::Data::CreateMesh(const Polyline& bound, const Polyline& outerBound) const
{
	if (plane) {
		if (bound.PointNum() == 0 && outerBound.PointNum() == 0) { return Mesh(); }
		auto normal = glm::normalize(plane->axis.second->data.Normal());
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
		auto normal = cylinder->axis.second->data.Normal();
		//if (!orient) { normal = -normal; }
		auto edge = SearchCylinderEdge(cylinder);
		Vector3 origin = cylinder->axis.second->data.point +
			normal * edge.minZ.first;
		//if (!orient) { begin = -begin; end = -end; }
		return Cylinder::CreateSideMesh(
			origin,
			normal,
			edge.begin,
			edge.end,
			cylinder->rad,
			edge.GetHeight(),
			orient,
			CIRCLE_SUBDIVISION_NUM,
			CIRCLE_SUBDIVISION_NUM);
	} else if (conical) {
		auto edge = SearchConicalEdge(conical);
		return Cone::CreateSideMesh(
			conical->axis.second->data.point,
			conical->axis.second->data.Normal(),
			edge.begin,edge.end,
			conical->radius,
			edge.GetHeight(),
			conical->angle,
			!orient,
			CIRCLE_SUBDIVISION_NUM,
			CIRCLE_SUBDIVISION_NUM);
	} else if (toroidal) {
		auto edge = SearchToroidalEdge(toroidal);
		auto normal = toroidal->axis.second->data.Normal();
		auto u = toroidal->axis.second->data.U();
		return Torus::CreateMesh(
			toroidal->axis.second->data.point,
			edge.vDir ? normal : normal,
			edge.uDir ? -u : u,
			toroidal->majorRadius,
			toroidal->minorRadius,
			edge.uBegin, edge.uEnd,
			edge.vBegin, edge.vEnd,
			orient,
			CIRCLE_SUBDIVISION_NUM,
			CIRCLE_SUBDIVISION_NUM);
	} else if(bSplineSurfaceWithKnots){
		return bSplineSurfaceWithKnots->CreateMesh();
	} else if (bSplineSurface) {
		return bSplineSurface->CreateMesh();
	}

	return Mesh();
}

STEPAxis2Placement3D* STEPFaceBase::GetAxis() const
{
	if (data.plane) {
		return data.plane->axis.second;
	}

	if (data.cylinder) {
		return data.cylinder->axis.second;
	}

	if (data.conical) {
		return data.conical->axis.second;
	}

	if (data.toroidal) {
		return data.toroidal->axis.second;
	}

	return nullptr;
}
void STEPFaceBase::FetchData(const STEPStruct& step)
{
	for (auto i = 0; i < raw.faceRef0.size(); i++) {
		auto pFaceOuterBound = FindSetData2(step, step.faceOuterBound, raw.faceRef0[i]);
		if (pFaceOuterBound) { data.faceOuterBound.push_back(pFaceOuterBound); continue; }
		auto pFaceBound = FindSetData2(step, step.faceBound, raw.faceRef0[i]);
		if (pFaceBound) { data.faceBound.push_back(pFaceBound); continue; }
	}
	data.plane = FindSetData2(step, step.planes, raw.geomRef1);
	if (!data.plane) {
		data.cylinder = FindSetData2(step, step.cylindricalSurface, raw.geomRef1);
	}
	if (!data.plane && !data.cylinder) {
		data.conical = FindSetData2(step, step.conicalSurface, raw.geomRef1);
	}
	if (!data.plane && !data.cylinder && !data.conical) {
		data.toroidal = FindSetData2(step, step.toroidalSurface, raw.geomRef1);
	}

	if (!data.plane && !data.cylinder && !data.conical && !data.toroidal) {
		data.bSplineSurfaceWithKnots = FindSetData2(step, step.bSplineSurfaceWithKnots, raw.geomRef1);
	}

	if (!data.plane && !data.cylinder && !data.conical && !data.toroidal && !data.bSplineSurfaceWithKnots) {
		data.bSplineSurface = FindSetData2(step, step.bSplineSurface, raw.geomRef1);
	}

	data.orient = raw.orient;
}

String STEPFaceBase::Dump(const DebugOption& option)
{
	auto str = ToString();
	for (auto& outerBound : data.faceOuterBound) { str += outerBound->Dump(option); }
	for (auto& faceBound : data.faceBound) { str += faceBound->Dump(option); }
	if (data.plane) { str += data.plane->Dump(option); }
	if (data.cylinder) { str += data.cylinder->Dump(option); }
	if (data.conical) { str += data.conical->Dump(option); }
	if (data.toroidal) { str += data.toroidal->Dump(option); }
	if (data.bSplineSurfaceWithKnots) { str += data.bSplineSurfaceWithKnots->Dump(option); }
	if (data.bSplineSurface) { str += data.bSplineSurface->Dump(option); }
	return str;
}

void STEPFaceBase::ShowUI(STEPUIContext& ui)
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
		if (data.conical) { data.conical->ShowUI(ui); }
		if (data.toroidal) { data.toroidal->ShowUI(ui); }
		if (data.bSplineSurfaceWithKnots) { data.bSplineSurfaceWithKnots->ShowUI(ui); }
		if (data.bSplineSurface) { data.bSplineSurface->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPFaceSurface::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto pData = new STEPFaceSurface();
	STEPEntityBase::Fetch(pData, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	auto faces = STEPString::SplitValue(values[1]);
	pData->raw.faceRef0.resize(faces.size());
	for (int i = 0; i < faces.size(); i++) {
		if (!STEPString::ValueToRef(faces[i], pData->raw.faceRef0[i])) { assert(0); return; }
	}

	if (!STEPString::ValueToRef(values[2], pData->raw.geomRef1)) { assert(0); return; }
	if (!STEPString::ValueToBool(values[3], pData->raw.orient)) { assert(0); return; }
	step.faceSurface[pData->id] = pData;
}


void STEPAdvancedFace::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPAdvancedFace();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	auto faces = STEPString::SplitValue(values[1]);
	data->raw.faceRef0.resize(faces.size());
	for (int i = 0; i < faces.size(); i++) {
		if (!STEPString::ValueToRef(faces[i], data->raw.faceRef0[i])) { assert(0); return; }
	}

	if (!STEPString::ValueToRef(values[2], data->raw.geomRef1)) { assert(0); return; }
	if (!STEPString::ValueToBool(values[3], data->raw.orient)) { assert(0); return; }
	step.advancedFace[data->id] = data;
}



void STEPShell::Fetch(STEPStruct& step, const STEPString& stepStr, STEPShell* pShell)
{
	auto values = STEPString::SplitValue(stepStr.entity.value);
	values = STEPString::SplitValue(values[1]);
	pShell->raw.faceRef.resize(values.size());
	for (int i = 0; i < values.size(); i++) {
		if (!STEPString::ValueToRef(values[i], pShell->raw.faceRef[i])) { assert(0); return; }
	}
	STEPEntityBase::Fetch(pShell, stepStr);
}

void STEPShell::FetchData(const STEPStruct& step, STEPShell::Data* data)
{
	data->Clear();
	for (auto i = 0; i < raw.faceRef.size(); i++) {
		auto pAdvanedFace = FindSetData2(step, step.advancedFace, raw.faceRef[i]);
		if (pAdvanedFace) { data->advancedFace.push_back(pAdvanedFace); continue; }

		auto pFaceSurface = FindSetData2(step, step.faceSurface, raw.faceRef[i]);
		if (pFaceSurface) { data->faceSurface.push_back(pFaceSurface); }
	}
}

void STEPShell::CreateMesh(const STEPShell& step, STEPShape& shape)
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

String STEPShell::Dump(const DebugOption& option)
{
	auto str = ToString();

	for (auto& face : data.advancedFace) { str += face->Dump(option); }
	for (auto& surface : data.faceSurface) { str += surface->Dump(option); }

	return str;
}

void STEPShell::ShowUI(STEPUIContext& ui)
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

void STEPClosedShell::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPClosedShell();
	STEPShell::Fetch(step, stepStr, data);
	step.closedShell[data->id] = data;
}

void STEPClosedShell::FetchData(const STEPStruct& step)
{
	STEPShell::FetchData(step, &data);
}

STEPShape STEPClosedShell::CreateMesh(const STEPStruct& step)
{
	FetchData(step);
	STEPShape mesh;
	STEPShell::CreateMesh(*this, mesh);
	return mesh;
}

void STEPOpenShell::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPOpenShell();
	STEPShell::Fetch(step, stepStr, data);
	step.openShell[data->id] = data;
}

void STEPOpenShell::FetchData(const STEPStruct& step)
{
	STEPShell::FetchData(step, &data);
}

STEPShape STEPOpenShell::CreateMesh(const STEPStruct& step)
{
	FetchData(step);
	STEPShape shape;
	STEPShell::CreateMesh(*this, shape);
	return shape;
}

void STEPQuasiUniformCurve::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPQuasiUniformCurve();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToInt(values[1], data->degree)) { assert(0); return; }
	auto split = STEPString::SplitValue(values[2]);
	data->points.resize(split.size());
	for (size_t i = 0; i < split.size(); i++) {
		if (!STEPString::ValueToRef(split[i], data->points[i].first)) { assert(0); return; }
	}
	if (!STEPString::ValueToBSplineCurveForm(values[3], data->form)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[4], data->closed)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[5], data->intersect)) { assert(0); return; }
	step.quasiUniformCurve[data->id] = data;
}

Polyline STEPQuasiUniformCurve::CreatePolyline(const Vector3& begin, const Vector3& end) const
{
	if (degree == 1) {
		Polyline line;
		for (size_t i = 0; i < points.size() - 1; i++) {
			line.Add(points[i].second->pos);
			line.Add(points[i + 1].second->pos);
		}
		return line;
	}

	assert(0);
	return Polyline();
}
void STEPQuasiUniformCurve::FetchData(const STEPStruct& step)
{
	for (size_t i = 0; i < points.size(); i++) {
		points[i].second = FindSetData2(step, step.points, points[i].first);
		if (!points[i].second) { assert(0); continue; }
	}
}
String STEPQuasiUniformCurve::Dump(const DebugOption& option)
{
	auto str = ToString();
	for (size_t i = 0; i < points.size(); i++) {
		str += points[i].second->Dump(option);
	}

	return str;
}
void STEPQuasiUniformCurve::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		for (size_t i = 0; i < points.size(); i++) {
			if (points[i].second) { points[i].second->ShowUI(ui); }
		}
		ImGui::TreePop();
	}
}

void STEPBSplineCurve::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	if (stepStr.multiEntity.empty()) { assert(0); return; }
	auto data = new STEPBSplineCurve();
	STEPEntityBase::Fetch(data, stepStr);
	for (size_t i = 0; i < stepStr.multiEntity.size(); i++) {
		const auto& entity = stepStr.multiEntity[i];
		if (StringUtility::Equal(entity.name, STEPBSplineCurveWithNotEntityName)) {
			data->FetchKnot(entity.value);
		} else if (StringUtility::Equal(entity.name, STEPRationalBSplineCurveEntityName)) {
			data->FetchRational(entity.value);
		} else if (StringUtility::Equal(entity.name, STEPBSplineCurve::EntityName)) {
			data->Fetch(entity.value);
		} else {
			if (StringUtility::Equal(entity.value, "()") ||
				StringUtility::Equal(entity.value, "('')")) {
				// value 無視パターン
			} else {
				assert(0);
			}
		}
	}

	step.bSplineCurve[data->id] = data;
}

void STEPBSplineCurve::Fetch(const String& str)
{
	auto values = STEPString::SplitValue(str);
	if (!STEPString::ValueToInt(values[0], degree)) { assert(0); return; }
	auto split = STEPString::SplitValue(values[1]);
	points.resize(split.size());
	for (size_t i = 0; i < split.size(); i++) {
		if (!STEPString::ValueToRef(split[i], points[i].first)) { assert(0); return; }
	}
	if (!STEPString::ValueToBSplineCurveForm(values[2], form)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[3], closed)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[4], intersect)) { assert(0); return; }

}

void STEPBSplineCurve::FetchKnot(const String& str)
{
	auto values = STEPString::SplitValue(str);
	auto multStr = STEPString::SplitValue(values[0]);
	for (size_t i = 0; i < multStr.size(); i++) {
		int multi = 0;
		if (!STEPString::ValueToInt(multStr[i], multi)) { assert(0); continue; }
		multiple.push_back(multi);
	}

	auto knotStr = STEPString::SplitValue(values[1]);
	for (size_t i = 0; i < knotStr.size(); i++) {
		float knot = 0;
		if (!STEPString::ValueToFloat(knotStr[i], knot)) { assert(0); continue; }
		knots.push_back(knot);
	}

	if (multiple.size() != knots.size()) {
		assert(0);
	}

	for (size_t i = 0; i < multiple.size(); i++) {
		for (size_t j = 0; j < multiple[i]; j++) {
			expandKnots.push_back(knots[i]);
		}
	}

	if (!STEPString::ValueToKnot(values.back(), knotType)) { assert(0); return; }
}

void STEPBSplineCurve::FetchRational(const String& str)
{
	auto values = STEPString::SplitValue(str);
	if (values.empty()) { return; }
	if (values[0].empty()) { return; }
	if (values[0][0] == '(') {
		values = STEPString::SplitValue(values[0]);
	}
	rational.resize(values.size());
	for (size_t i = 0; i < rational.size(); i++) {
		if (!STEPString::ValueToFloat(values[i], rational[i])) { assert(0); continue; }
	}
}

Polyline STEPBSplineCurve::CreatePolyline(const Vector3& begin, const Vector3& end) const
{
	return STEP::BSplineBuilder::CreatePolyline(*this, CIRCLE_SUBDIVISION_NUM, begin, end);
}
void STEPBSplineCurve::FetchData(const STEPStruct& step)
{
	for (size_t i = 0; i < points.size(); i++) {
		points[i].second = FindSetData2(step, step.points, points[i].first);
		if (!points[i].second) { assert(0); continue; }
	}
}

String STEPBSplineCurve::Dump(const DebugOption& option)
{
	auto str = ToString();
	for (size_t i = 0; i < points.size(); i++) {
		str += points[i].second->Dump(option);
	}

	return str;
}
void STEPBSplineCurve::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		for (size_t i = 0; i < points.size(); i++) {
			if (points[i].second) { points[i].second->ShowUI(ui); }
		}
		ImGui::TreePop();
	}
}

void STEPBSplineSurfaceWithKnots::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPBSplineSurfaceWithKnots();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.entity.value);
	if (!STEPString::ValueToInt(values[1], data->degree.u)) { assert(0); return; }
	if (!STEPString::ValueToInt(values[2], data->degree.v)) { assert(0); return; }
	auto split = STEPString::SplitValue(values[3]);
	data->points.resize(split.size());
	for (size_t i = 0; i < split.size(); i++) {
		auto uvStr = STEPString::SplitValue(split[i]);
		if (!STEPString::ValueToRef(uvStr[0], data->points[i].first.u)) { assert(0); return; }
		if (!STEPString::ValueToRef(uvStr[1], data->points[i].first.v)) { assert(0); return; }
	}
	if (!STEPString::ValueToBSplineSurfaceForm(values[4], data->form)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[5], data->closed.u)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[6], data->closed.v)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[7], data->intersect)) { assert(0); return; }
	step.bSplineSurfaceWithKnots[data->id] = data;

}
void STEPBSplineSurfaceWithKnots::FetchData(const STEPStruct& step)
{
	for (size_t i = 0; i < points.size(); i++) {
		points[i].second.u = FindSetData2(step, step.points, points[i].first.u);
		if (!points[i].second.u) { assert(0); continue; }
		points[i].second.v = FindSetData2(step, step.points, points[i].first.v);
		if (!points[i].second.v) { assert(0); continue; }
	}
}
String STEPBSplineSurfaceWithKnots::Dump(const DebugOption& option)
{
	auto str = ToString();
	for (size_t i = 0; i < points.size(); i++) {
		str += points[i].second.u->Dump(option);
		str += points[i].second.v->Dump(option);
	}

	return str;
}
void STEPBSplineSurfaceWithKnots::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		for (size_t i = 0; i < points.size(); i++) {
			if (points[i].second.u) { points[i].second.u->ShowUI(ui); }
			if (points[i].second.v) { points[i].second.v->ShowUI(ui); }
		}
		ImGui::TreePop();
	}
}

Mesh STEPBSplineSurfaceWithKnots::CreateMesh() const
{
	return Mesh();
}




void STEPBSplineSurface::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPBSplineSurface();
	STEPEntityBase::Fetch(data, stepStr);
	for (size_t i = 0; i < stepStr.multiEntity.size(); i++) {
		const auto& entity = stepStr.multiEntity[i];
		if (StringUtility::Equal(entity.name, STEPBSplineSurfaceWithKnots::EntityName)) {
			data->FetchKnot(entity.value);
		} else if (StringUtility::Equal(entity.name, STEPRationalBSplineSurfaceEntityName)) {
			data->FetchRational(entity.value);
		} else if (StringUtility::Equal(entity.name, STEPBSplineSurface::EntityName)) {
			data->Fetch(entity.value);
		} else {
			if (StringUtility::Equal(entity.value, "()") ||
				StringUtility::Equal(entity.value, "('')")) {
				// value 無視パターン
			} else {
				assert(0);
			}
		}
	}

	step.bSplineSurface[data->id] = data;
}

void STEPBSplineSurface::Fetch(const String& str)
{
	auto values = STEPString::SplitValue(str);
	if (!STEPString::ValueToInt(values[0], degree.u)) { assert(0); return; }
	if (!STEPString::ValueToInt(values[1], degree.v)) { assert(0); return; }
	auto split = STEPString::SplitValue(values[2]);
	points.resize(split.size());
	for (size_t i = 0; i < split.size(); i++) {
		auto uvStr = STEPString::SplitValue(split[i]);
		points[i].resize(uvStr.size());
		for (size_t j = 0; j < uvStr.size(); j++) {
			if (!STEPString::ValueToRef(uvStr[j], points[i][j].first)) { assert(0); return; }
		}
	}
	if (!STEPString::ValueToBSplineSurfaceForm(values[3], form)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[4], closed.u)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[5], closed.v)) { assert(0); return; }
	if (!STEPString::ValueToLogical(values[6], intersect)) { assert(0); return; }
}
void STEPBSplineSurface::FetchKnot(const String& str)
{
	auto values = STEPString::SplitValue(str);
	auto split = STEPString::SplitValue(values[0]);
	multiple.u.resize(split.size());
	for (size_t i = 0; i < split.size(); i++) {
		if (!STEPString::ValueToInt(split[i], multiple.u[i])) { assert(0); return; }
	}

	split = STEPString::SplitValue(values[1]);
	multiple.v.resize(split.size());
	for (size_t i = 0; i < split.size(); i++) {
		if (!STEPString::ValueToInt(split[i], multiple.v[i])) { assert(0); return; }
	}

	split = STEPString::SplitValue(values[2]);
	knots.u.resize(split.size());
	for (size_t i = 0; i < split.size(); i++) {
		if (!STEPString::ValueToFloat(split[i], knots.u[i])) { assert(0); return; }
	}

	split = STEPString::SplitValue(values[3]);
	knots.v.resize(split.size());
	for (size_t i = 0; i < split.size(); i++) {
		if (!STEPString::ValueToFloat(split[i], knots.v[i])) { assert(0); return; }
	}

	if (!STEPString::ValueToKnot(values[4], knotType)) { assert(0); return; }
}
void STEPBSplineSurface::FetchRational(const String& str)
{
	auto values = STEPString::SplitValue(str);
	if (values.empty()) { return; }
	if (values[0].empty()) { return; }
	if (values[0][0] == '(') {
		values = STEPString::SplitValue(values[0]);
	}
	rational.resize(values.size());
	for (size_t i = 0; i < rational.size(); i++) {
		auto uvStr = STEPString::SplitValue(values[i]);
		rational[i].resize(uvStr.size());
		for (size_t j = 0; j < uvStr.size(); j++) {
			if (!STEPString::ValueToFloat(uvStr[j], rational[i][j])) { assert(0); continue; }
		}
	}
}

bool STEPBSplineSurface::IsValid() const
{
	if (GetUNum() == 0) { return false; }
	if (GetVNum() == 0) { return false; }

	for (int i = 1; i < GetUNum(); ++i) {
		if ((int)points[i].size() != GetVNum()) {
			return false;
		}
	}

	if (rational.empty()) { return true; }
	if ((int)rational.size() != GetUNum()) {
		return false;
	}
	for (int i = 0; i < GetUNum(); ++i) {
		if ((int)rational[i].size() != GetVNum()) {
			return false;
		}
	}

}
Mesh STEPBSplineSurface::CreateMesh() const
{
	return STEP::BSplineBuilder::CreateMesh(*this, CIRCLE_SUBDIVISION_NUM, CIRCLE_SUBDIVISION_NUM);
}
void STEPBSplineSurface::FetchData(const STEPStruct& step)
{
	for (size_t i = 0; i < points.size(); i++) {
		for (size_t j = 0; j < points[i].size(); j++) {
			points[i][j].second = FindSetData2(step, step.points, points[i][j].first);
			if (!points[i][j].second) { assert(0); continue; }
		}
	}
}
String STEPBSplineSurface::Dump(const DebugOption& option)
{
	auto str = ToString();
	for (size_t i = 0; i < points.size(); i++) {
		for (size_t j = 0; j < points[i].size(); j++) {
			str += points[i][j].second->Dump(option);
		}
	}

	return str;
}
void STEPBSplineSurface::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		for (size_t i = 0; i < points.size(); i++) {
			for (size_t j = 0; j < points[i].size(); j++) {
				if (points[i][j].second) { points[i][j].second->ShowUI(ui); }
			}
		}
		ImGui::TreePop();
	}
}
}
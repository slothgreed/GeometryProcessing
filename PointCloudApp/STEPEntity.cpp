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
	step.name = StringUtility::TrimWhiteSpace(nameToValue.first);
	step.value = '(' + nameToValue.second;
	return step;
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
	auto valuePoint = StringUtility::After(value, ',');
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

String STEPString::ToString() const
{
	return
		"#" + StringUtility::ToString(id) +
		", " + name +
		", " + value;
}

STEPStruct::~STEPStruct()
{
	for (auto& v : points) { delete v.second; }
	for (auto& v : lines) { delete v.second; }
	for (auto& v : circles) { delete v.second; }
	for (auto& v : planes) { delete v.second; }
	for (auto& v : vectors) { delete v.second; }
	for (auto& v : directions) { delete v.second; }
	for (auto& v : edgeCurve) { delete v.second; }
	for (auto& v : axis2Placement3D) { delete v.second; }
	for (auto& v : cylindricalSurface) { delete v.second; }
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
	auto values = STEPString::SplitValue(stepStr.value);
	values = STEPString::SplitValue(values[1]);
	if (!STEPString::ValueToFloat(values[0], data->data.pos.x)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[1], data->data.pos.y)) { assert(0); return; }
	if (values.size() == 3) {
		if (!STEPString::ValueToFloat(values[2], data->data.pos.z)) { assert(0); return; }
	}
	step.points[data->id] = data;
}

void STEPPoint::FetchData(const STEPStruct& step) {}

void STEPPoint::Printf(const DebugOption& option) { STEPEntityBase::PrintfRaw(); }
void STEPPoint::ShowUI(STEPUIContext& ui) { ShowLeaf(ui); }

void STEPDirection::Fetch(STEPStruct& step, const STEPString& stepStr)
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

void STEPDirection::FetchData(const STEPStruct& step) {}
void STEPDirection::Printf(const DebugOption& option) { STEPEntityBase::PrintfRaw(); }
void STEPDirection::ShowUI(STEPUIContext& ui) { ShowLeaf(ui); }

void STEPVector::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPVector();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.value);
	if (!STEPString::ValueToRef(values[1], data->raw.idRef)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[2], data->raw.length)) { assert(0); return; }

	step.vectors[data->id] = data;
}

void STEPVector::FetchData(const STEPStruct& step)
{
	data.driection = FindSetData2(step, step.directions, raw.idRef);
	data.vector = data.driection->direction;
	data.vector = glm::normalize(data.vector);
	data.vector *= raw.length;
}

void STEPVector::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.driection) { data.driection->Printf(option); }
}

void STEPVector::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.driection) { data.driection->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPLine::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPLine();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.value);
	if (!STEPString::ValueToRef(values[1], data->raw.beginRef)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[2], data->raw.vectorRef)) { assert(0); return; }
	step.lines[data->id] = data;
}

void STEPLine::FetchData(const STEPStruct& step)
{
	data.point = FindSetData2(step, step.points, raw.beginRef);
	data.begin = data.point->data.pos;

	data.vector0 = FindSetData2(step, step.vectors, raw.vectorRef);
	data.vector = data.vector0->data.vector;
}

Polyline STEPLine::Data::CreatePolyline()
{
	return Polyline(Vector<Vector3>{begin, begin + vector});
}

void STEPLine::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.point) { data.point->Printf(option); }
	if (data.vector0) { data.vector0->Printf(option); }
}


void STEPLine::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.point) { data.point->ShowUI(ui); }
		if (data.vector0) { data.vector0->ShowUI(ui); }
		ImGui::TreePop();
	}
}


void STEPAxis2Placement3D::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPAxis2Placement3D();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.value);
	if (!STEPString::ValueToRef(values[1], data->raw.pointRef)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[2], data->raw.dirRef1)) { assert(0); return; }
	if (!STEPString::ValueToRef(values[3], data->raw.dirRef2)) { assert(0); return; }

	step.axis2Placement3D[data->id] = data;
}

void STEPAxis2Placement3D::FetchData(const STEPStruct& step)
{
	data.point0 = FindSetData2(step, step.points, raw.pointRef);
	data.point = data.point0->data.pos;

	data.direction1 = FindSetData2(step, step.directions, raw.dirRef1);
	if (data.direction1) data.dir1 = data.direction1->direction;


	data.direction2 = FindSetData2(step, step.directions, raw.dirRef2);
	if (data.direction2) data.dir2 = data.direction2->direction;
}

void STEPAxis2Placement3D::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.point0) { data.point0->Printf(option); }
	if (data.direction1) { data.direction1->Printf(option); }
	if (data.direction2) { data.direction2->Printf(option); }
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


Polyline STEPCircle::Data::CreatePolyline() const
{
	auto v = glm::cross(axis->data.dir1, axis->data.dir2);
	return Circle::CreateLine(rad, CIRCLE_SUBDIVISION_NUM,
		glm::normalize(axis->data.dir2), glm::normalize(v), axis->data.point);
}

Polyline STEPCircle::Data::CreatePolyline(const Vector3& begin, const Vector3& end) const
{
	auto v = glm::cross(axis->data.dir1, axis->data.dir2);
	return Circle::CreateArc(rad, CIRCLE_SUBDIVISION_NUM,
		glm::normalize(axis->data.dir2), glm::normalize(v), axis->data.point, begin, end);
}

void STEPCircle::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPCircle();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.value);
	if (!STEPString::ValueToRef(values[1], data->raw.axisRef)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[2], data->raw.rad)) { assert(0); return; }
	step.circles[data->id] = data;
}

void STEPCircle::FetchData(const STEPStruct& step)
{
	data.axis = FindSetData2(step, step.axis2Placement3D, raw.axisRef);
	data.rad = raw.rad;
}

void STEPCircle::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.axis) { data.axis->Printf(option); }
}

void STEPCircle::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.axis) { data.axis->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPCylindricalSurface::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPCylindricalSurface();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.value);
	if (!STEPString::ValueToRef(values[1], data->raw.axisRef)) { assert(0); return; }
	if (!STEPString::ValueToFloat(values[2], data->raw.rad)) { assert(0); return; }
	step.cylindricalSurface[data->id] = data;
}

void STEPCylindricalSurface::FetchData(const STEPStruct& step)
{
	data.axis = FindSetData2(step, step.axis2Placement3D, raw.axisRef);
	data.rad = raw.rad;
}

void STEPCylindricalSurface::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.axis) { data.axis->Printf(option); }
}

void STEPCylindricalSurface::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.axis) { data.axis->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPConicalSurface::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPConicalSurface();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.value);
	if (!STEPString::ValueToRef(values[1], data->raw.idRef)) { assert(0); return; }
	step.conicalSurface[data->id] = data;
}

void STEPConicalSurface::FetchData(const STEPStruct& step)
{
	data.axis = FindSetData2(step, step.axis2Placement3D, raw.idRef);
}

void STEPConicalSurface::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.axis) { data.axis->Printf(option); }
}

void STEPConicalSurface::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.axis) { data.axis->ShowUI(ui); }
		ImGui::TreePop();
	}
}

void STEPPlane::Fetch(STEPStruct& step, const STEPString& stepStr)
{
	auto data = new STEPPlane();
	STEPEntityBase::Fetch(data, stepStr);
	auto values = STEPString::SplitValue(stepStr.value);
	if (!STEPString::ValueToRef(values[1], data->raw.idRef)) { assert(0); return; }
	step.planes[data->id] = data;
}

void STEPPlane::FetchData(const STEPStruct& step)
{
	data.axis = FindSetData2(step, step.axis2Placement3D, raw.idRef);
}

void STEPPlane::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.axis) { data.axis->Printf(option); }
}

void STEPPlane::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.axis) { data.axis->ShowUI(ui); }
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
	auto values = STEPString::SplitValue(stepStr.value);
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


void STEPInterSectionCurve::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.curve0.pLine) { data.curve0.pLine->Printf(option); }
	if (data.curve0.pCircle) { data.curve0.pCircle->Printf(option); }
	if (data.surf0.pPlane) { data.surf0.pPlane->Printf(option); }
	if (data.surf1.pCylinderSurface) { data.surf1.pCylinderSurface->Printf(option); }
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
	auto values = STEPString::SplitValue(stepStr.value);
	if (!STEPString::ValueToRef(values[1], data->raw.idRef)) { assert(0); return; }
	step.vertexPoint[data->id] = data;
}

void STEPVertexPoint::FetchData(const STEPStruct& step)
{
	data.point = FindSetData2(step, step.points, raw.idRef);
}

void STEPVertexPoint::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.point) { data.point->Printf(option); }
}

void STEPVertexPoint::ShowUI(STEPUIContext& ui)
{
	if (ShowBranch(ui, ui.IsSelect(id))) {
		if (data.point) { data.point->ShowUI(ui); }
		ImGui::TreePop();
	}
}

Polyline STEPEdgeCurve::CreatePolyline() const
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

void STEPEdgeCurve::Fetch(STEPStruct& step, const STEPString& stepStr)
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

void STEPEdgeCurve::FetchData(const STEPStruct& step)
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

void STEPEdgeCurve::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.pPoint0) { data.pPoint0->Printf(option); }
	if (data.pPoint1) { data.pPoint1->Printf(option); }
	if (data.line) { data.line->Printf(option); }
	if (data.circle) { data.circle->Printf(option); }
	if (data.intersectionCurve) { data.intersectionCurve->Printf(option); }
}


void STEPEdgeCurve::ShowUI(STEPUIContext& ui)
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
	auto values = STEPString::SplitValue(stepStr.value);
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

void STEPOrientedEdge::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.edgeCurve) { data.edgeCurve->Printf(option); }
	if (data.vertex0) { data.vertex0->Printf(option); }
	if (data.vertex1) { data.vertex1->Printf(option); }
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
		lines[2 * i] = points[i]->data.pos;
		lines[2 * i + 1] = points[(i + 1) % points.size()]->data.pos;
	}
	return PolylineList(id, Polyline(std::move(lines), Polyline::DrawType::Lines));
}


void STEPPolyLoop::Fetch(STEPStruct& step, const STEPString& stepStr)
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

void STEPPolyLoop::FetchData(const STEPStruct& step)
{
	data.points.resize(raw.idRef.size());
	for (auto i = 0; i < raw.idRef.size(); i++) {
		data.points[i] = FindSetData2(step, step.points, raw.idRef[i]);
	}
}

void STEPPolyLoop::Printf(const DebugOption& option)
{
	PrintfRaw();
	for (size_t i = 0; i < data.points.size(); i++) {
		data.points[i]->Printf(option);
	}
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
	auto values = STEPString::SplitValue(stepStr.value);
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

void STEPEdgeLoop::Printf(const DebugOption& option)
{
	PrintfRaw();
	for (auto orientedEdge : data.orientedEdges) {
		if (orientedEdge == nullptr) { continue; }
		orientedEdge->Printf(option);
	}
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
	auto values = STEPString::SplitValue(stepStr.value);
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

void STEPFaceBoundBase::Printf(const DebugOption& option)
{
	PrintfRaw();
	if (data.edgeLoop) { data.edgeLoop->Printf(option); }
	if (data.polyLoop) { data.polyLoop->Printf(option); }
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

STEPFaceBase::Data::CylidnerEdge STEPFaceBase::Data::SearchCylinderEdge(const STEPCylindricalSurface* pCylinder) const
{
	CylidnerEdge ret;
	bool setBegin = false; bool setEnd = false;
	for (const auto& face : faceBound) {
		auto edgeLoop = face->data.edgeLoop;
		if (!edgeLoop) { continue; }
		for (const auto& edge : edgeLoop->data.orientedEdges) {
			auto begin = edge->data.GetBegin() - pCylinder->data.axis->data.point;
			auto end = edge->data.GetEnd() - pCylinder->data.axis->data.point;
			auto v0 = glm::dot(begin, pCylinder->data.axis->data.Normal());
			auto v1 = glm::dot(end, pCylinder->data.axis->data.Normal());

			auto sameBeginDir = MathHelper::IsSameDir(edge->data.GetBegin() - edge->data.GetEnd(), pCylinder->data.axis->data.Normal());
			auto sameEndDir = MathHelper::IsSameDir(edge->data.GetEnd() - edge->data.GetBegin(), pCylinder->data.axis->data.Normal());
			//if (sameBeginDir || sameEndDir)
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
			auto begin = edge->data.GetBegin() - pCylinder->data.axis->data.point;
			auto end = edge->data.GetEnd() - pCylinder->data.axis->data.point;
			auto v0 = glm::dot(begin, pCylinder->data.axis->data.Normal());
			auto v1 = glm::dot(end, pCylinder->data.axis->data.Normal());

			auto sameBeginDir = MathHelper::IsSameDir(edge->data.GetBegin() - edge->data.GetEnd(), pCylinder->data.axis->data.Normal());
			auto sameEndDir = MathHelper::IsSameDir(edge->data.GetEnd() - edge->data.GetBegin(), pCylinder->data.axis->data.Normal());
			//if (sameBeginDir || sameEndDir)
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
		ret.begin = pCylinder->data.axis->data.point + pCylinder->data.axis->data.U() * pCylinder->data.rad;
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
		auto edge = SearchCylinderEdge(cylinder);
		if (!edge.IsActive()) { return Mesh(); }
		Vector3 origin = cylinder->data.axis->data.point +
			normal * edge.minZ.first;
		//if (!orient) { begin = -begin; end = -end; }
		return Cylinder::CreateSideMesh(
			origin,
			normal,
			edge.begin,
			edge.end,
			cylinder->data.rad,
			edge.GetHeight(),
			orient,
			CIRCLE_SUBDIVISION_NUM,
			CIRCLE_SUBDIVISION_NUM);
	} else if (conical) {
		// TODO;
		int a = 0;
	}

	return Mesh();
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
	data.orient = raw.orient;
}

void STEPFaceBase::Printf(const DebugOption& option)
{
	PrintfRaw();

	for (auto& outerBound : data.faceOuterBound) { outerBound->Printf(option); }
	for (auto& faceBound : data.faceBound) { faceBound->Printf(option); }
	if (data.plane) { data.plane->Printf(option); }
	if (data.cylinder) { data.cylinder->Printf(option); }
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

		ImGui::TreePop();
	}
}

void STEPFaceSurface::Fetch(STEPStruct& step, const STEPString& stepStr)
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


void STEPAdvancedFace::Fetch(STEPStruct& step, const STEPString& stepStr)
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



void STEPShell::Fetch(STEPStruct& step, const STEPString& stepStr, STEPShell* pShell)
{
	auto values = STEPString::SplitValue(stepStr.value);
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

void STEPShell::Printf(const DebugOption& option)
{
	PrintfRaw();

	for (auto& face : data.advancedFace) { face->Printf(option); }
	for (auto& surface : data.faceSurface) { surface->Printf(option); }
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
}
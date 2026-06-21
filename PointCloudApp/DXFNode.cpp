#include "DXFNode.h"
#include "ShaderTable.h"
#include "SimpleShader.h"
#include "FileUtility.h"
#include "Primitives.h"
namespace KI
{

	
DXFStruct* DXFLoader::Load(const String& name)
{
	Vector<String> lines;
	if (!FileUtility::Load(name, lines)) {
		Assert::Failed();
		return nullptr;
	}
	DebugPrintf::StringStr("Load", name); DebugPrintf::NewLine();


	auto dxf = new DXFStruct();
	for (size_t i = 0; i < lines.size(); i += 2) {
		auto code = StringUtility::ToInt(lines[i]);
		auto value = StringUtility::TrimWhiteSpace(lines[i + 1]);
		if (code == DXF_SYSTEM_VARIABLE) { continue; }
		else if (code == DXF_DESCRIPTION) { continue; }
		else if (code == DXF_ENTITY) {
			if (ToDXFRecordType(value) == DXFRecordType::Line) {
				dxf->lines.push_back(ParseLine(lines, i));
			} else if (ToDXFRecordType(value) == DXFRecordType::LType) {
				dxf->ltype.push_back(ParseLType(lines, i));
			} else if (ToDXFRecordType(value) == DXFRecordType::Text) {
				dxf->texts.push_back(ParseText(lines, i));
			} else if (ToDXFRecordType(value) == DXFRecordType::Polyline) {
				dxf->polylines.push_back(ParsePolyLine(lines, i));
			} else if (ToDXFRecordType(value) == DXFRecordType::Arc) {
				dxf->arcs.push_back(ParseArc(lines, i));
			} else if (ToDXFRecordType(value) == DXFRecordType::Point) {
				dxf->points.push_back(ParsePoint(lines, i));
			}
		} else {
			DebugPrintf::StringStr("Unknown code: " + StringUtility::ToString(code) + " value: " + value + "\n");
		}
		dxf->codeValue.push_back(std::make_pair(code, value));
	}


	return dxf;
}	

DXFPoint DXFLoader::ParsePoint(const Vector<String>& lines, size_t& index)
{
	// VertexÇ∆ìØìôÇ»ÇÃÇ≈ÅAParseVertexÇóòópÇ∑ÇÈÅB
	auto vertex = ParseVertex(lines, index);
	DXFPoint point;
	point.layerName = vertex.layerName;
	point.point = vertex.point;
	return point;
}

DXFArc DXFLoader::ParseArc(const Vector<String>& lines, size_t& index)
{
	DXFArc arc;
	index = index + 2;
	for (size_t i = index; i < lines.size(); i += 2) {
		auto code = StringUtility::ToInt(lines[i]);
		if (code == 0) { break; }
		auto value = StringUtility::TrimWhiteSpace(lines[i + 1]);
		if (code == DXF_LAYER_NAME) { arc.layerName = value; } 
		else if (code == DXF_X0) { arc.center.x = StringUtility::ToFloat(value); }
		else if (code == DXF_Y0) { arc.center.y = StringUtility::ToFloat(value); }
		else if (code == DXF_Z0) { arc.center.z = StringUtility::ToFloat(value); }
		else if (code == DXF_RADIUS) { arc.radius = StringUtility::ToFloat(value); } 
		else if (code == DXF_BEGIN_ANGLE) { arc.beginAngle = StringUtility::ToFloat(value); } 
		else if (code == DXF_END_ANGLE) { arc.endAngle = StringUtility::ToFloat(value); } 
		else {
			DebugPrintf::StringStr("Unknown code: " + StringUtility::ToString(code) + " value: " + value + "\n");
		}
		index = i;
	}

	return arc;
}
DXFPolyline DXFLoader::ParsePolyLine(const Vector<String>& lines, size_t& index)
{
	DXFPolyline polyline;
	
	enum class NEXT_ENTITY
	{
		NEXT_ENTITY_UNKNOWN = 0,
		NEXT_ENTITY_VERTEX = 1
	};

	index = index + 2;
	for (size_t i = index; i < lines.size(); i += 2) {
		auto code = StringUtility::ToInt(lines[i]);
		auto value = StringUtility::TrimWhiteSpace(lines[i + 1]);
		if (code == 0) {
			if (DXFRecordType::Vertex == ToDXFRecordType(value)) {
				polyline.vertexs.push_back(ParseVertex(lines, i));
			} else {
				break;
			}
		}
		if (code == DXF_LAYER_NAME) { polyline.layerName = value; }
		else if (code == DXF_X0) { polyline.point.x = StringUtility::ToFloat(value); }
		else if (code == DXF_Y0) { polyline.point.y = StringUtility::ToFloat(value); }
		else if (code == DXF_Z0) { polyline.point.z = StringUtility::ToFloat(value); }
		else if (code == DXF_COLOR) { polyline.color = StringUtility::ToInt(value); } 
		else if (code == DXF_LINE_TYPE) { polyline.lineType = ToDXFLineType(value); }
		else if (code == DXF_FLAG) { polyline.flag = StringUtility::ToInt(value); }
		else {
			DebugPrintf::StringStr("Unknown code: " + StringUtility::ToString(code) + " value: " + value + "\n");
		}
		index = i;
	}
	return polyline;
}

DXFText DXFLoader::ParseText(const Vector<String>& lines, size_t& index)
{
	DXFText text;
	index = index + 2;
	for(size_t i = index; i < lines.size();i += 2) {
		auto code = StringUtility::ToInt(lines[i]);
		if (code == 0) { break; }
		// TODO ;
		index = i;
	}
	return text;
}
DXFVertex DXFLoader::ParseVertex(const Vector<String>& lines, size_t& index)
{
	DXFVertex vertex;
	index = index + 2;
	for (size_t i = index; i < lines.size(); i += 2) {
		auto code = StringUtility::ToInt(lines[i]);
		if (code == 0) { break; }
		auto value = StringUtility::TrimWhiteSpace(lines[i + 1]);
		if (code == DXF_LAYER_NAME) { vertex.layerName = value; }
		else if (code == DXF_X0) { vertex.point.x = StringUtility::ToFloat(value); }
		else if (code == DXF_Y0) { vertex.point.y = StringUtility::ToFloat(value); }
		else if (code == DXF_Z0) { vertex.point.z = StringUtility::ToFloat(value); }
		else {
			DebugPrintf::StringStr("Unknown code: " + StringUtility::ToString(code) + " value: " + value + "\n");
		}
		index = i;
	}

	return vertex;
}

DXFLType DXFLoader::ParseLType(const Vector<String>& lines, size_t& index)
{
	DXFLType ltype;
	index = index + 2;
	for (size_t i = index; i < lines.size(); i += 2) {
		auto code = StringUtility::ToInt(lines[i]);
		if (code == 0) { break; }
		// TODO ;
		index = i;
	}
	return ltype;
}
DXFLine DXFLoader::ParseLine(const Vector<String>& lines, size_t& index)
{
	DXFLine line;
	index = index + 2;
	for (int i = index; i < lines.size(); i += 2) {
		auto code = StringUtility::ToInt(lines[i]);
		if (code == 0) { break; }
		auto value = StringUtility::TrimWhiteSpace(lines[i + 1]);
		if (code == DXF_LAYER_NAME) { line.layerName = value; }
		else if (code == DXF_X0) { line.begin.x = StringUtility::ToFloat(value); }
		else if (code == DXF_Y0) { line.begin.y = StringUtility::ToFloat(value); }
		else if (code == DXF_Z0) { line.begin.z = StringUtility::ToFloat(value); }
		else if (code == DXF_X1) { line.end.x = StringUtility::ToFloat(value); }
		else if (code == DXF_Y1) { line.end.y = StringUtility::ToFloat(value); }
		else if (code == DXF_Z1) { line.end.z = StringUtility::ToFloat(value); }
		else {
			DebugPrintf::StringStr("Unknown code: " + StringUtility::ToString(code) + " value: " + value + "\n");
		}

		index = i;
	}

	return line;
}
void DXFNode::DrawNode(const DrawContext& context)
{
	BuildGLBuffer();
	const auto& pResource = context.pResource;
	auto pCamera = pResource->GetCameraBuffer();

	auto pSimpleShader = pResource->GetShaderTable()->GetSimpleShader();
	pSimpleShader->Use();
	pSimpleShader->SetCamera(pCamera);
	pSimpleShader->SetModel(GetMatrix());

	if (m_pLines) {
		pSimpleShader->SetColor(Vector3(1, 0, 0));
		pSimpleShader->SetPosition(m_pLines.get());
		pSimpleShader->DrawArray(GL_LINES, m_pLines->Num());
	}


	if (m_pPoints) {
		pSimpleShader->SetColor(Vector3(0, 1, 0));
		pSimpleShader->SetPosition(m_pPoints.get());
		pSimpleShader->DrawArray(GL_POINTS, m_pPoints->Num());
	}


	pSimpleShader->SetColor(Vector3(0, 0, 1));
	for(size_t i = 0; i < m_pPolylines.size(); i++) {
		pSimpleShader->SetPosition(m_pPolylines[i].get());
		pSimpleShader->DrawArray(GL_LINE_STRIP, m_pPolylines[i]->Num());
	}

	pSimpleShader->SetColor(Vector3(0, 1, 1));
	for (size_t i = 0; i < m_pArcs.size(); i++) {
		pSimpleShader->SetPosition(m_pArcs[i].get());
		pSimpleShader->DrawArray(GL_LINES, m_pArcs[i]->Num());
	}

}

DXFNode::DXFNode(const String& name, const Shared<DXFStruct>& pDXF) 
	:RenderNode(name)
	, m_pDXF(pDXF)
{
	BDB bdb;
	for (size_t i = 0; i < m_pDXF->lines.size(); i++) {
		bdb.Add(m_pDXF->lines[i].begin);
		bdb.Add(m_pDXF->lines[i].end);
	}
	SetBoundBox(bdb);
}
void DXFNode::ShowUI(UIContext& ui)
{
	ImGui::BeginChild("CodeToValue", ImVec2(0, 0), true);
	for (int i = 0; i < m_pDXF->codeValue.size(); i++) {
		if(m_pDXF->codeValue[i].first == DXF_ENTITY) {
			ImGui::Separator();
		}
		auto str = StringUtility::ToString(m_pDXF->codeValue[i].first) + ": " + m_pDXF->codeValue[i].second;
		ImGui::Text(str.c_str());
	}

	ImGui::EndChild();

}	

void DXFNode::BuildGLBuffer()
{
	if (m_pLines || m_pPoints || m_pArcs.size() != 0 || m_pPolylines.size() != 0) { return; }

	if (m_pDXF->lines.size()) {
		m_pLines = std::make_unique<GLBuffer>();
		Vector<Vector3> points;
		for (size_t i = 0; i < m_pDXF->lines.size(); i++) {
			points.push_back(m_pDXF->lines[i].begin);
			points.push_back(m_pDXF->lines[i].end);
		}
		m_pLines->Create(points);
	}

	if (m_pDXF->points.size()) {
		Vector<Vector3> points;
		for (size_t i = 0; i < m_pDXF->points.size(); i++) {
			points.push_back(m_pDXF->points[i].point);
		}
		m_pPoints = std::make_unique<GLBuffer>();
		m_pPoints->Create(points);
	}

	if (m_pDXF->polylines.size()) {
		for (size_t i = 0; i < m_pDXF->polylines.size(); i++) {
			Vector<Vector3> points;
			for (size_t j = 0; j < m_pDXF->polylines[i].vertexs.size(); j++) {
				points.push_back(m_pDXF->polylines[i].vertexs[j].point);
			}
			auto pPolyline = std::make_unique<GLBuffer>();
			pPolyline->Create(points);
			m_pPolylines.push_back(std::move(pPolyline));
		}
	}


	if (m_pDXF->arcs.size()) {
		for (size_t i = 0; i < m_pDXF->arcs.size(); i++) {
			const auto& arc = m_pDXF->arcs[i];
			auto polyline = Circle::CreateArc(arc.radius, 36, arc.center, arc.beginAngle, arc.endAngle);
			auto pArc = std::make_unique<GLBuffer>();
			pArc->Create(polyline.GetPoints());
			m_pArcs.push_back(std::move(pArc));
		}
	}
}
}
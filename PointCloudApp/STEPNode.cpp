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
#include "PrimitiveNode.h"
#include "STEPEntity.h"
#include <functional>
namespace KI
{

void NotDefineEntity(const String& str)
{
	// 形状に不必要なEntity
	// マテリアル情報も含まれる。
	static const std::vector<std::string> equals = {
		"ACTION_DIRECTIVE",
		"ACTION_METHOD",
		"ANGULAR_LOCATION",
		"APPLIED_AREA",
		"CHAMFER",
		"CHAMFER_OFFSET",
		"CIRCULAR_CLOSED_PROFILE",
		"COLOUR_RGB",
		"COORDINATED_UNIVERSAL_TIME_OFFSET",
		"DERIVED_UNIT",
		"DERIVED_UNIT_ELEMENT",
		"DESCRIPTIVE_REPRESENTATION_ITEM",
		"DESIGN_CONTEXT",
		"DIRECTED_ACTION",
		"DRAUGHTING_PRE_DEFINED_COLOUR",
		"FACE_SHAPE_REPRESENTATION",
		"FILL_AREA_STYLE",
		"FILL_AREA_STYLE_COLOUR",
		"HOLE_BOTTOM",
		"INSTANCED_FEATURE",
		"LENGTH_MEASURE_WITH_UNIT",
		"LOCAL_TIME",
		"MACHINING_FEATURE",
		"MAKE_FROM_USAGE_OPTION",
		"MATERIAL_DESIGNATION",
		"MEASURE_QUALIFICATION",
		"MEASURE_REPRESENTATION_ITEM",
		"MEASURE_WITH_UNIT",
		"ORDERED_PART",
		"ORGANIZATION",
		"ORGANIZATION_ROLE",
		"OUTER_ROUND",
		"PATH_FEATURE_COMPONENT",
		"PLANE_ANGLE_MEASURE_WITH_UNIT",
		"PLUS_MINUS_TOLERANCE",
		"PRECISION_QUALIFIER",
		"PRESENTATION_STYLE_ASSIGNMENT",
		"QUALIFIED_REPRESENTATION_ITEM",
		"REPRESENTATION",
		"REPRESENTATION_ITEM",
		"REVOLVED_PROFILE",
		"ROUND_HOLE",
		"SHAPE_ASPECT",
		"SHAPE_ASPECT_RELATIONSHIP",
		"SHAPE_DEFINING_RELATIONSHIP",
		"SHAPE_DEFINITION_REPRESENTATION",
		"SHAPE_DIMENSION_REPRESENTATION",
		"SHAPE_REPRESENTATION_RELATIONSHIP",
		"SHAPE_REPRESENTATION_WITH_PARAMETERS",
		"SQUARE_U_PROFILE",
		"STANDARD_UNCERTAINTY",
		"STYLED_ITEM",
		"SURFACE_SIDE_STYLE",
		"SURFACE_STYLE_FILL_AREA",
		"SURFACE_STYLE_USAGE",
		"THREAD",
		"TOLERANCE_VALUE",
		"VEE_PROFILE",
		"VERSIONED_ACTION_REQUEST",
	};

	static const std::vector<std::string> contains = {
		"APPROVAL",
		"APPLICATION",
		"FEATURE",
		"PRODUCT",
		"PERSON",
		"CC_DESIGN",
		"DIMENSIONAL",
		"DATUM",
		"DATA",
		"DATE",
		"MECHANICAL",
		"PROPERTY",
		"SECURITY",
		"MEASURE"
	};

	"LENGTH_MEASURE_WITH_UNIT";
	"ADVANCED_BREP_SHAPE_REPRESENTATION";
	"MANIFOLD_SOLID_BREP";
	"UNCERTAINTY_MEASURE_WITH_UNIT";
	"SHAPE_DEFINITION_REPRESENTATION";
	"SHAPE_REPRESENTATION_RELATIONSHIP";

	auto stepStr = STEPString::Create(str);
	for (const auto& key : equals) {
		if (StringUtility::Equal(stepStr.name, key)) {
			return;
		}
	}

	for (const auto& key : contains) {
		if (StringUtility::Contains(str, key)) {
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
		if (contents[i].empty())continue;
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
		else if (StringUtility::Equal(stepStr.name, STEPCylindricalSurface::EntityName)) { STEPCylindricalSurface::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPInterSectionCurve::EntityName)) { STEPInterSectionCurve::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPConicalSurface::EntityName)) { STEPConicalSurface::Fetch(step, stepStr); }
		else if (StringUtility::Equal(stepStr.name, STEPToroidalSurface::EntityName)) { STEPToroidalSurface::Fetch(step, stepStr); }
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

	auto pAxis = pFace->GetAxis();
	if (pAxis) {
		auto len = BDB(mesh.GetPoints()).MaxLength();
		auto axis = std::make_shared<Axis>(pAxis->data.point, pAxis->data.U() * len, pAxis->data.V() * len, pAxis->data.Normal() * len);
		context.ui->AddDebugNode(std::make_shared<PrimitiveNode>(pBase->str + "::Axis", axis));
	}
	
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
	uiContext.filePath = GetName();
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
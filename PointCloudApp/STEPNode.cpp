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

STEPStruct* STEPLoader::Load(const String& name, bool saveOriginal)
{
	auto extension = FileUtility::GetExtension(name);
	if (!(extension == ".step" || extension == ".stp")) { return nullptr; }
	Vector<String> contents;
	if (!FileUtility::Load(name, contents)) {
		assert(0, "Failed to load STEP file: " + name);
		return nullptr;
	}
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

	auto pStep = new STEPStruct();
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

		auto multiEntityType = stepStr.GetMultiEntityType();
		if (multiEntityType != ESTEPNone) {
			if (multiEntityType == ESTEPBSplineCurve) {
				STEPBSplineCurve::Fetch(*pStep, stepStr);
			} else if (multiEntityType == ESTEPBSplineSurface) {
				STEPBSplineSurface::Fetch(*pStep, stepStr);
			}
		}
		else if (StringUtility::Equal(stepStr.entity.name, STEPPoint::EntityName)) { STEPPoint::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPDirection::EntityName)) { STEPDirection::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPVector::EntityName)) { STEPVector::Fetch(*pStep, stepStr);}
		else if (StringUtility::Equal(stepStr.entity.name, STEPPlane::EntityName)) { STEPPlane::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPLine::EntityName)) { STEPLine::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPAxis2Placement3D::EntityName)) { STEPAxis2Placement3D::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPEdgeCurve::EntityName)) { STEPEdgeCurve::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPVertexPoint::EntityName)) { STEPVertexPoint::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPEdgeLoop::EntityName)) { STEPEdgeLoop::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPPolyLoop::EntityName)) { STEPPolyLoop::Fetch(*pStep, stepStr); } 
		else if (StringUtility::Equal(stepStr.entity.name, STEPFaceBound::EntityName)) { STEPFaceBound::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPFaceOuterBound::EntityName)) { STEPFaceOuterBound::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPOrientedEdge::EntityName)) { STEPOrientedEdge::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPAdvancedFace::EntityName)) { STEPAdvancedFace::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPFaceSurface::EntityName)) { STEPFaceSurface::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPClosedShell::EntityName)) { STEPClosedShell::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPOpenShell::EntityName)) { STEPOpenShell::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPCircle::EntityName)) { STEPCircle::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPCylindricalSurface::EntityName)) { STEPCylindricalSurface::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPInterSectionCurve::EntityName)) { STEPInterSectionCurve::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPConicalSurface::EntityName)) { STEPConicalSurface::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPToroidalSurface::EntityName)) { STEPToroidalSurface::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPQuasiUniformCurve::EntityName)) { STEPQuasiUniformCurve::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPBSplineSurfaceWithKnots::EntityName)) { STEPBSplineSurfaceWithKnots::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPPolyLine::EntityName)) { STEPPolyLine::Fetch(*pStep, stepStr); } 
		else if (StringUtility::Equal(stepStr.entity.name, STEPManifoldSolidBrep::EntityName)) { STEPManifoldSolidBrep::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPAdvancedBrepShapeRepresentation::EntityName)) { STEPAdvancedBrepShapeRepresentation::Fetch(*pStep, stepStr); }
		else if (StringUtility::Equal(stepStr.entity.name, STEPShapeRepresentation::EntityName)) { STEPShapeRepresentation::Fetch(*pStep, stepStr); } 
		else if (StringUtility::Equal(stepStr.entity.name, STEPShapeRepresentationRelationShip::EntityName)) { STEPShapeRepresentationRelationShip::Fetch(*pStep, stepStr); }
		else { STEPEntityBase::NotDefineEntity(content); writeEntity = false; }

		if (writeEntity && saveOriginal) {
			writer.Write(content, true);
		}
	}

	if (saveOriginal) {
		writer.Close();
	}

	return pStep;
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
void STEPRenderNode::BuildShape()
{
	if (!m_step) { return; }
	Vector<STEPShape> shapes;
	for (const auto& shapeRepresentation : m_step->shapeRepresentationRelationShip) {
		shapeRepresentation.second->FetchData(*m_step);
	}
	for (const auto& shell : m_step->closedShell) {
		if (m_step->shapeRepresentationRelationShip.size() == 0) {
			shell.second->FetchData(*m_step);
		}
		shapes.push_back(shell.second->CreateMesh(*m_step));
	}

	for (const auto& shell : m_step->openShell) {
		if (m_step->shapeRepresentationRelationShip.size() == 0) {
			shell.second->FetchData(*m_step);
		}
		shapes.push_back(shell.second->CreateMesh(*m_step));
	}

	m_shape = std::move(shapes);
	BuildGLResource();
}

void STEPRenderNode::BuildGLResource()
{
	BuildGPUResource();
	BuildNode();
}

void STEPRenderNode::BuildGPUResource()
{
	if (m_shape.size() == 0) { return; }
	if (m_gpu.size() != 0) { return; }

	for (size_t i = 0; i < m_shape.size(); i++) {
		const auto& shape = m_shape[i];
		GPUShell gpu;
		auto cube = Cube::CreateLine(shape.GetBDB().Min(), shape.GetBDB().Max());
		gpu.bdb.drawType = GL_LINES;
		gpu.bdb.pPosition = std::make_unique<GLBuffer>();
		gpu.bdb.pIndex = std::make_unique<GLBuffer>();
		gpu.bdb.pPosition->Create(cube.Position());
		gpu.bdb.pIndex->Create(cube.Index());

		for (const auto& m : shape.meshs) {
			const auto& mesh = m.second;
			if (mesh.GetIndexs().size() != 0) {
				gpu.triangleIndex.pointNum += mesh.GetPoints().size();
				gpu.triangleIndex.indexNum += mesh.GetIndexs().size();
			} else {
				gpu.triangle.pointNum += mesh.GetPoints().size();
			}
		}

		for (const auto& pList : shape.polylineList) {
			for (const auto& p : pList.GetPolylines()) {
				const auto& polyline = p.second;
				if (polyline.GetPoints().size() == 0) { continue; }
				if (polyline.GetDrawType() == GL_LINES) {
					if (polyline.GetIndexs().size() != 0) {
						gpu.lineIndex.pointNum += polyline.GetPoints().size();
						gpu.lineIndex.indexNum += polyline.GetIndexs().size();
					} else {
						gpu.line.pointNum += polyline.GetPoints().size();
					}
				} else 	if (polyline.GetDrawType() == GL_LINE_STRIP) {
					if (polyline.GetIndexs().size() != 0) {
						gpu.lineStripIndex.pointNum += polyline.GetPoints().size();
						gpu.lineStripIndex.indexNum += polyline.GetIndexs().size();
					} else {
						gpu.lineStrip.pointNum += polyline.GetPoints().size();
					}
				} else 	if (polyline.GetDrawType() == GL_LINE_LOOP) {
					if (polyline.GetIndexs().size() != 0) {
						gpu.lineLoopIndex.pointNum += polyline.GetPoints().size();
						gpu.lineLoopIndex.indexNum += polyline.GetIndexs().size();
					} else {
						gpu.lineLoop.pointNum += polyline.GetPoints().size();
					}
				}
			}
		}

		gpu.triangle.Allocate(GL_TRIANGLES);	gpu.triangleIndex.Allocate(GL_TRIANGLES);
		gpu.line.Allocate(GL_LINES);	gpu.lineIndex.Allocate(GL_LINES);
		gpu.lineStrip.Allocate(GL_LINE_STRIP); gpu.lineStripIndex.Allocate(GL_LINE_STRIP);
		gpu.lineLoop.Allocate(GL_LINE_LOOP); gpu.lineLoop.Allocate(GL_LINE_LOOP);
		gpu.pShape = &m_shape[i];
		m_gpu[m_shape[i].id] = std::move(gpu);
	}

	for (size_t i = 0; i < m_gpu.size(); i++) {
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

		auto& gpu = m_gpu[m_shape[i].id];
		auto& shape = *gpu.pShape;
		for (const auto& m : shape.meshs) {
			const auto& mesh = m.second;
			if (mesh.GetPoints().size() == 0) { continue; }
			if (mesh.GetIndexs().size()) {
				gpu.triangleIndex.pPosition->BufferSubData(triangleIndexPointOffset, mesh.GetPoints());
				triangleIndexPointOffset += mesh.GetPoints().size();

				gpu.triangleIndex.AddEntity(m.first, triangleIndexOffset, mesh.GetIndexs().size());
				gpu.triangleIndex.pIndex->BufferSubData(triangleIndexOffset, mesh.GetIndexs());
				triangleIndexOffset += mesh.GetIndexs().size();
			} else {
				gpu.triangle.AddEntity(m.first, trianglePointOffset, mesh.GetPoints().size());
				gpu.triangle.pPosition->BufferSubData(trianglePointOffset, mesh.GetPoints());
				trianglePointOffset += mesh.GetPoints().size();
			}
		}

		for (const auto& pList : shape.polylineList) {
			for (const auto& p : pList.GetPolylines()) {
				const auto& polyline = p.second;
				if (polyline.GetPoints().size() == 0) { continue; }
				if (polyline.GetDrawType() == GL_LINES) {
					if (polyline.GetIndexs().size()) {
						gpu.lineIndex.pPosition->BufferSubData(lineIndexPointOffset, polyline.GetPoints());
						lineIndexPointOffset += polyline.GetPoints().size();

						gpu.lineIndex.AddEntity(p.first, lineIndexOffset, polyline.GetIndexs().size());
						gpu.lineIndex.pIndex->BufferSubData(lineIndexOffset, polyline.GetIndexs());
						lineIndexOffset += polyline.GetIndexs().size();
					} else {
						gpu.line.AddEntity(p.first, linePointOffset, polyline.GetPoints().size());
						gpu.line.pPosition->BufferSubData(linePointOffset, polyline.GetPoints());
						linePointOffset += polyline.GetPoints().size();
					}
				} else if (polyline.GetDrawType() == GL_LINE_STRIP) {
					if (polyline.GetIndexs().size()) {
						gpu.lineStripIndex.pPosition->BufferSubData(lineStripIndexPointOffset, polyline.GetPoints());
						lineStripIndexPointOffset += polyline.GetPoints().size();

						gpu.lineStripIndex.AddEntity(p.first, lineStripIndexOffset, polyline.GetIndexs().size());
						gpu.lineStripIndex.pIndex->BufferSubData(lineStripIndexOffset, polyline.GetIndexs());
						lineStripIndexOffset += polyline.GetIndexs().size();
					} else {
						gpu.lineStrip.AddEntity(p.first, lineStripPointOffset, polyline.GetPoints().size());
						gpu.lineStrip.pPosition->BufferSubData(lineStripPointOffset, polyline.GetPoints());
						lineStripPointOffset += polyline.GetPoints().size();
					}
				} else if (polyline.GetDrawType() == GL_LINE_LOOP) {
					if (polyline.GetIndexs().size()) {
						gpu.lineLoopIndex.pPosition->BufferSubData(lineLoopIndexPointOffset, polyline.GetPoints());
						lineLoopIndexPointOffset += polyline.GetPoints().size();

						gpu.lineLoopIndex.AddEntity(p.first, lineLoopIndexOffset, polyline.GetIndexs().size());
						gpu.lineLoopIndex.pIndex->BufferSubData(lineLoopIndexOffset, polyline.GetIndexs());
						lineLoopIndexOffset += polyline.GetIndexs().size();
					} else {
						gpu.lineLoop.AddEntity(p.first, lineLoopPointOffset, polyline.GetPoints().size());
						gpu.lineLoop.pPosition->BufferSubData(lineLoopPointOffset, polyline.GetPoints());
						lineLoopPointOffset += polyline.GetPoints().size();
					}
				}
			}
		}
	}
}

void STEPRenderNode::BuildNode()
{
	if (m_root.child.size() != 0) { return; }
	Vector<Node> nodes;
	BDB bdb;
	if (m_step->shapeRepresentationRelationShip.size() == 0) {
		Node node;
		for (const auto& shell : m_step->closedShell) {
			auto pShell = &m_gpu[shell.first];
			node.gpuShell.push_back(pShell);
			bdb.Add(pShell->pShape->GetBDB());
		}

		for (const auto& shell : m_step->openShell) {
			auto pShell = &m_gpu[shell.first];
			node.gpuShell.push_back(pShell);
			bdb.Add(pShell->pShape->GetBDB());
		}
		nodes.push_back(std::move(node));
	} else {
		for (const auto& relationShip : m_step->shapeRepresentationRelationShip) {
			Node node;
			const auto& representation = relationShip.second->shapeRepresentation;
			if (representation) {
				node.world = representation->axis2Placement3D.front().second->CreateMatrix();
			}
			const auto& advanced = relationShip.second->advancedBrepShapeRepresentation;
			if (advanced) {
				for (const auto& solid : advanced->manifoldSolidBrep) {
					if (solid.second) {
						auto pShell = &m_gpu[solid.second->shell.first];
						node.gpuShell.push_back(pShell);
						bdb.Add(pShell->pShape->GetBDB().CreateRotate(node.world));
					}
				}
			}
			nodes.push_back(std::move(node));
		}
	}
	m_root.child = std::move(nodes);
	SetBoundBox(bdb);
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
	if (m_gpu.size() == 0) { return; }
	auto pResource = context.pResource;
	m_root.world = GetTranslateMatrix() * GetScaleMatrix() * m_rotateMatrix;
	auto pSimpleShader = pResource->GetShaderTable()->GetSimpleShader().get();
	pSimpleShader->Use();
	pSimpleShader->SetCamera(context.pResource->GetCameraBuffer());
	for (const auto& node : m_root.child) {
		pSimpleShader->SetModel(m_root.world * node.world);
		for (const auto& shell : node.gpuShell) {
			DrawShell(context, pSimpleShader, *shell);
		}
	}
}

void STEPRenderNode::DrawShell(const DrawContext& context, SimpleShader* pSimpleShader, const GPUShell& shell)
{
	Vector3 selectColor = Vector3(1.0f, 1.0f, 1.0f);
	if (m_ui.visibleBDB) {
		pSimpleShader->SetColor(Vector3(0, 0, 1));
		pSimpleShader->SetPosition(shell.bdb.pPosition.get());
		pSimpleShader->DrawElement(shell.bdb.drawType, shell.bdb.pIndex.get());
	}

	if (m_ui.visibleWire) {
		if (shell.line.pPosition) {
			pSimpleShader->SetPosition(shell.line.pPosition.get());
			pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
			pSimpleShader->DrawArray(shell.line.drawType, shell.line.pPosition.get());
			auto entity = shell.line.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawArray(shell.line.drawType, entity.first, entity.num);
			}
		}

		if (shell.lineStrip.pPosition) {
			pSimpleShader->SetPosition(shell.lineStrip.pPosition.get());
			pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
			pSimpleShader->DrawArray(shell.lineStrip.drawType, shell.lineStrip.pPosition.get());
			auto entity = shell.lineStrip.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawArray(shell.lineStrip.drawType, entity.first, entity.num);
			}
		}

		if (shell.lineLoop.pPosition) {
			pSimpleShader->SetPosition(shell.lineLoop.pPosition.get());
			pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
			pSimpleShader->DrawArray(shell.lineLoop.drawType, shell.lineLoop.pPosition.get());
			auto entity = shell.lineLoop.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawArray(shell.lineLoop.drawType, entity.first, entity.num);
			}
		}

		if (shell.lineIndex.pPosition) {
			pSimpleShader->SetPosition(shell.lineIndex.pPosition.get());
			pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
			pSimpleShader->DrawElement(shell.lineIndex.drawType, shell.lineIndex.pIndex.get());
			auto entity = shell.lineIndex.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawArray(shell.lineIndex.drawType, entity.first, entity.num);
			}
		}

		if (shell.lineStripIndex.pPosition) {
			pSimpleShader->SetPosition(shell.lineStripIndex.pPosition.get());
			pSimpleShader->SetColor(Vector3(0.0f, 0.0f, 0.0f));
			pSimpleShader->DrawElement(shell.lineStripIndex.drawType, shell.lineStripIndex.pIndex.get());
			auto entity = shell.lineStripIndex.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawArray(shell.lineStripIndex.drawType, entity.first, entity.num);
			}
		}

		if (shell.lineLoopIndex.pPosition) {
			pSimpleShader->SetPosition(shell.lineLoopIndex.pPosition.get());
			pSimpleShader->SetColor(Vector3(1.0f, 1.0f, 1.0f));
			pSimpleShader->DrawElement(shell.lineLoopIndex.drawType, shell.lineLoopIndex.pIndex.get());
			auto entity = shell.lineLoopIndex.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawArray(shell.lineLoopIndex.drawType, entity.first, entity.num);
			}
		}
	}

	if (m_ui.visibleMesh) {
		if (shell.triangle.pPosition) {
			pSimpleShader->SetPosition(shell.triangle.pPosition.get());
			pSimpleShader->SetColor(Vector3(1.0f, 0.0f, 0.0f));
			pSimpleShader->DrawArray(shell.triangle.drawType, shell.triangle.pPosition.get());
			auto entity = shell.triangle.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawArray(shell.triangle.drawType, entity.first, entity.num);
			}
		}

		if (shell.triangleIndex.pPosition) {
			pSimpleShader->SetPosition(shell.triangleIndex.pPosition.get());
			pSimpleShader->SetColor(Vector3(0.0f, 1.0f, 0.0f));
			pSimpleShader->DrawElement(shell.triangleIndex.drawType, shell.triangleIndex.pIndex.get());
			auto entity = shell.triangleIndex.FindEntity(uiContext.GetSelectId());
			if (entity.IsActive()) {
				pSimpleShader->SetColor(selectColor);
				pSimpleShader->DrawElement(shell.triangleIndex.drawType, shell.triangleIndex.pIndex.get(), entity.num, entity.first);
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
	ImGui::Checkbox("VisibleWire", &m_ui.visibleWire);
	ImGui::Checkbox("VisibleMesh", &m_ui.visibleMesh);
	if (ImGui::TreeNode("Root")) {
		for (auto& node : m_step->shapeRepresentationRelationShip) { node.second->ShowUI(uiContext); }
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

	// âÒì]Çí~êœÇ∑ÇÈèÍçá
	m_rotateMatrix = translateBack * rotate * translateToOrigin * m_rotateMatrix;
}

}
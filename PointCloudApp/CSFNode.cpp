#include "CSFNode.h"
#include "SimpleShader.h"
#define CSF_IMPLEMENTATION
#include "CSFNode.h"
#include "Utility.h"
#include <fileformats/cadscenefile.h>
namespace KI
{

RenderNode* CSFLoader::Load(const String& fileName)
{
    CSFile* csf;
    CSFileMemoryPTR mem = CSFileMemory_new();
    if (CSFile_loadExt(&csf, fileName.data(), mem) != CADSCENEFILE_NOERROR || !(csf->fileFlags & CADSCENEFILE_FLAG_UNIQUENODES)) {
        CSFileMemory_delete(mem);
        return nullptr;
    }
    CSFile_transform(csf);

    Vector<KI::CSFMaterial> materials(csf->numMaterials);
    for (auto i = 0; i < materials.size(); i++) {
        materials[i].diffuse = Vector4(
            csf->materials[i].color[0],
            csf->materials[i].color[1],
            csf->materials[i].color[2],
            csf->materials[i].color[3]);
		materials[i].roughness = 0.4f;
		materials[i].metallic = 0.0f;
    }


     std::vector<CSFVertex> vertexs;
    std::vector<std::vector<CSFDrawRange>> solidDraw(csf->numGeometries);
    std::vector<std::vector<CSFDrawRange>> wiresDraw(csf->numGeometries);
    std::vector<GLuint> indices;
    size_t wireOffset = 0;
    size_t solidOffset = 0;
    size_t vertexOffset = 0;
    auto pMeshBuffer = std::make_unique<CSFMeshBuffer>();
    for (int i = 0; i < csf->numGeometries; i++) {
        auto csfGeom = &csf->geometries[i];
        for (int j = 0; j < csfGeom->numVertices; j++) {
            CSFVertex vertex{};
            vertex.position = Vector4(
                csfGeom->vertex[3 * j + 0],
                csfGeom->vertex[3 * j + 1],
                csfGeom->vertex[3 * j + 2], 1.0f);
			pMeshBuffer->bdb.Add(Vector3(vertex.position.x, vertex.position.y, vertex.position.z));
            if (csfGeom->normal) {
                vertex.normal = Vector4(
                    csfGeom->normal[3 * j + 0],
                    csfGeom->normal[3 * j + 1],
                    csfGeom->normal[3 * j + 2], 0.0f);
            } else {
                vertex.normal = glm::normalize(vertex.position);
                vertex.normal.w = 0.0f;
            }

            vertexs.push_back(vertex);
        }

        int offset = indices.size();
        indices.resize(indices.size() + csfGeom->numIndexSolid + csfGeom->numIndexWire);
        memcpy(&indices[offset], csfGeom->indexSolid, sizeof(GLuint)* csfGeom->numIndexSolid);
        if (csfGeom->indexWire) {
            memcpy(&indices[offset + csfGeom->numIndexSolid], csfGeom->indexWire, sizeof(GLuint)* csfGeom->numIndexWire);
            // solidの後にwire用のIndexBufferが構築されるのでここで可算する
            wireOffset += csfGeom->numIndexSolid * sizeof(GLuint);
        }

        for (auto j = 0; j < csfGeom->numParts; j++) {
            CSFDrawRange solid;
            CSFDrawRange wire;

            solid.draw.count = csfGeom->parts[j].numIndexSolid;
            solid.draw.offset = solidOffset;
            solid.draw.baseVertex = vertexOffset;
            solid.draw.primitive = PRIMITIVE_TRIANGLE;
            solid.draw.dataType = DATA_UINT;

            wire.draw.count = csfGeom->parts[j].numIndexWire;
            wire.draw.offset = wireOffset;
            wire.draw.baseVertex = vertexOffset;
            wire.draw.primitive = PRIMITIVE_LINES;
            wire.draw.dataType = DATA_UINT;

            solidOffset += solid.draw.count * sizeof(GLuint);
            wireOffset += wire.draw.count * sizeof(GLuint);
            
            solidDraw[i].push_back(solid);
            wiresDraw[i].push_back(wire);
        }

        // wire分が足されていないのでここで加算する。
        solidOffset += csfGeom->numIndexWire * sizeof(GLuint);
        vertexOffset += csfGeom->numVertices;

    }
    pMeshBuffer->Init();
    pMeshBuffer->pVertex->Create<CSFVertex>(vertexs);
    pMeshBuffer->pIndex->Create(indices);
    Vector<Unique<CSFMeshBuffer>> meshBuffers;
    meshBuffers.push_back(std::move(pMeshBuffer));


    int numObject = 0;
    std::vector<KI::CSFNode> matrixs(csf->numNodes);
    std::vector<CSFSolid> solids;
    for (auto i = 0; i < csf->numNodes; i++) {
        auto node = &csf->nodes[i];

        memcpy(glm::value_ptr(matrixs[i].matrix), node->objectTM, sizeof(float) * 16);
        memcpy(glm::value_ptr(matrixs[i].world), node->worldTM, sizeof(float) * 16);

        matrixs[i].matrixIT = glm::transpose(glm::inverse(matrixs[i].matrix));
        matrixs[i].worldIT = glm::transpose(glm::inverse(matrixs[i].world));

		if (node->geometryIDX < 0) { continue; }

        for (int j = 0; j < node->numParts; j++) {
            CSFSolid  solid;
            solid.geomIndex = node->geometryIDX;
            solid.matrixIndex = node->parts[j].nodeIDX < 0 ? i : node->parts[j].nodeIDX;
            solid.materialIndex = node->parts[j].materialIDX;
            solid.drawIndex = j;
            solids.push_back(solid);
        }

        numObject++;
    }

    auto pNode = new CSFRenderNode(fileName);
    pNode->SetNode(std::move(matrixs));
    pNode->SetMaterial(std::move(materials));
    pNode->SetMeshBuffer(std::move(meshBuffers));
    pNode->SetWireDraw(std::move(wiresDraw));
    pNode->SetSolidDraw(std::move(solidDraw));
    pNode->SetSolids(std::move(solids));
    
    return pNode;
}
class CSFShader : public IShadingShader
{
public:
	CSFShader() {}
	virtual ~CSFShader() {}

	enum UNIFORM
	{
		NODE,
		MODEL,
		NUM
	};

	virtual ShaderPath GetShaderPath()
	{
		ShaderPath path;
		path.version = "version.h";
		path.header.push_back("common.h");
		path.header.push_back("pbr\\pbr.h");
		path.header.push_back("csf/csf.h");
		path.shader[SHADER_PROGRAM_VERTEX] = "csf/csf.vert";
		path.shader[SHADER_PROGRAM_FRAG] = "csf/csf.frag";
		
		return path;
	}

	void FetchUniformLocation()
	{
		m_uModel = GetUniformLocation("u_Model");
		m_uNode = GetUniformLocation("u_node");
		m_uPrefilter = GetUniformLocation("u_prefilter");
		m_uIrradiance = GetUniformLocation("u_irradiance");
		m_uBRDF = GetUniformLocation("u_brdf");
	}

	void SetCameraBuffer(const GLBuffer* pBuffer)
	{
		BindShaderStorage(0, pBuffer->Handle());
	}

	void SetLightBuffer(const GLBuffer* pBuffer)
	{
		BindShaderStorage(1, pBuffer->Handle());
	}

	void SetMaterialBuffer(const GLBuffer* pBuffer)
	{
		BindShaderStorage(2, pBuffer->Handle());
	}
	void SetNodeBuffer(const GLBuffer* pBuffer)
	{
		BindShaderStorage(3, pBuffer->Handle());
	}

	void SetPBRResource(const PBRResource* pPBR)
	{
		BindShaderStorage(4, pPBR->GetGlobalParam()->Handle());
		BindTexture(m_uBRDF, 5, *pPBR->GetBRDFLUT());
		BindCubemap(m_uIrradiance, 6, *pPBR->GetIrradiance());
		BindCubemap(m_uPrefilter, 7, *pPBR->GetPrefiltered());

	}
	void SetModel(const Matrix4x4& node)
	{
		BindUniform(m_uModel, node);
	}

	void SetNode(const Vector2i& node)
	{
		BindUniform(m_uNode, node);
	}

	void SetVertexBuffer(const GLBuffer* pBuffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, pBuffer->Handle());
		glBindVertexBuffer(0, pBuffer->Handle(), 0, pBuffer->SizeOfData());
		glEnableVertexAttribArray(0);
		glVertexAttribFormat(0, 4, GL_FLOAT, false, offsetof(CSFVertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(CSFVertex, normal));
		glVertexAttribBinding(0, 0);
		glVertexAttribBinding(1, 0);
		glDisableVertexAttribArray(2); // 仮コード/別箇所でAttributeがEnableになる。
		OUTPUT_GLERROR;
	}

	void Draw(const CSFDrawRange& drawRange)
	{
		if (drawRange.draw.count == 0) return;
		DrawElementsBaseVertex(drawRange.draw);
		OUTPUT_GLERROR;
	}
private:
	GLuint m_uModel;
	GLuint m_uNode;
	GLuint m_uPrefilter;
	GLuint m_uIrradiance;
	GLuint m_uBRDF;
};

const CSFRenderNode::CSFRenderParts* CSFRenderNode::CSFRenderParts::Cast(const RenderParts* pParts)
{
	assert(dynamic_cast<const CSFRenderNode::CSFRenderParts*>(pParts));
	return static_cast<const CSFRenderNode::CSFRenderParts*>(pParts);

}
String CSFRenderNode::CSFRenderParts::ToString()
{
	std::stringstream ss;
	
	if (type == Face) {
		ss << "Face :";
	} else if (type == Edge) {
		ss << "Edge :";
	}

	ss << parts << "\n";
	ss << "Geometry Index" << m_pNode->m_solids[parts].geomIndex << "\n";
	ss << "Matrix Index" << m_pNode->m_solids[parts].matrixIndex << "\n";
	ss << "Material Index" << m_pNode->m_solids[parts].materialIndex << "\n";
	return ss.str();

}
CSFRenderNode::CSFRenderNode(const String& name)
	: RenderNode(name)
	, m_pShader(nullptr)
	, m_needUpdateProperty(true)
{
};
CSFRenderNode::~CSFRenderNode()
{
	RELEASE_INSTANCE(m_pShader);
}

void CSFRenderNode::SetMeshBuffer(Vector<Unique<CSFMeshBuffer>>&& mesh)
{
	m_gpu.pMeshBuffer = std::move(mesh);
	BDB bdb;
	for (const auto& pBuffer : m_gpu.pMeshBuffer) {
		bdb.Add(pBuffer->bdb);
	}
	SetBoundBox(bdb);
}
void CSFRenderNode::UpdateProperty()
{
	if (!m_needUpdateProperty) { return; }
	m_property = Property();

	for (const auto& solid : m_solids) {
		m_property.triangleNum += m_solidDraw[solid.geomIndex][solid.drawIndex].draw.count;
		m_property.lineNum += m_wireDraw[solid.geomIndex][solid.drawIndex].draw.count;
	}

	m_property.triangleNum /= 3;
	m_property.lineNum /= 2;
	m_needUpdateProperty = true;
}
void CSFRenderNode::PickNode(const PickContext& context)
{
	if (!m_ui.visible) { return; }
	auto pResource = context.pResource;
	auto pPickShader = pResource->GetShaderTable()->GetPointPickByID();

	VertexFormat format;
	format.type = DATA_FLOAT;
	format.offset = offsetof(CSFVertex, position);
	format.normalized = false;
	format.componentSize = 4;

	pPickShader->Use();
	pPickShader->SetCamera(pResource->GetCameraBuffer());
	pPickShader->SetPosition(format, m_gpu.pMeshBuffer[0]->pVertex.get());
	pPickShader->BindIndexBuffer(m_gpu.pMeshBuffer[0]->pIndex.get());
	int i = 0;
	for (const auto& solid : m_solids) {
		pPickShader->SetModel(m_nodes[solid.matrixIndex].world);
		pPickShader->SetPickOffset(i++);
		pPickShader->DrawElementsBaseVertex(m_solidDraw[solid.geomIndex][solid.drawIndex].draw);
	}

	for (const auto& solid : m_solids) {
		pPickShader->SetModel(m_nodes[solid.matrixIndex].world);
		pPickShader->SetPickOffset(i++);
		pPickShader->DrawElementsBaseVertex(m_wireDraw[solid.geomIndex][solid.drawIndex].draw);
	}
}


bool CSFRenderNode::CollectPickedNode(PickResult& result)
{
	int resultId = result.id;
	if (resultId < m_solids.size()) {
		result.pResult[this] = std::make_unique<CSFRenderParts>(this, CSFRenderParts::Face, resultId);
		return true;
	}

	resultId = result.id - m_solids.size();
	if (resultId < m_solids.size()) {
		result.pResult[this] = std::make_unique<CSFRenderParts>(this, CSFRenderParts::Edge, resultId);
		return true;
	}

	return false;
}
void CSFRenderNode::DrawPartsNode(const DrawContext& context, const RenderParts& parts)
{
	auto pResource = context.pResource;
	auto drawParts = CSFRenderParts::Cast(&parts);
	m_pShader->Use();
	m_pShader->SetCameraBuffer(context.pResource->GetCameraBuffer());
	m_pShader->SetLightBuffer(context.pResource->GetLightBuffer());
	m_pShader->SetMaterialBuffer(m_gpu.pMaterialBuffer.get());
	m_pShader->SetNodeBuffer(m_gpu.pNodeBuffer.get());
	m_pShader->SetVertexBuffer(m_gpu.pMeshBuffer[0]->pVertex.get());
	m_pShader->BindIndexBuffer(m_gpu.pMeshBuffer[0]->pIndex.get());
	const auto& solid = m_solids[drawParts->parts];
	m_pShader->SetNode(Vector2i(solid.matrixIndex, solid.materialIndex));
	if (drawParts->type == CSFRenderParts::Face) {
		m_pShader->Draw(m_solidDraw[solid.geomIndex][solid.drawIndex]);
	} else {
		m_pShader->Draw(m_wireDraw[solid.geomIndex][solid.drawIndex]);
	}
}

void CSFRenderNode::UpdateMaterial()
{
	if (m_gpu.pMaterialBuffer == nullptr) {
		m_gpu.pMaterialBuffer = std::make_unique<GLBuffer>();
		m_gpu.pMaterialBuffer->Create<KI::CSFMaterial>(m_materials);
	}
}

void CSFRenderNode::UpdateNode()
{
	if (m_gpu.pNodeBuffer == nullptr) {
		m_gpu.pNodeBuffer = std::make_unique<GLBuffer>();
		m_gpu.pNodeBuffer->Create<KI::CSFNode>(m_nodes);
	}
}

void CSFRenderNode::DrawNode(const DrawContext& context)
{
	if (!m_ui.visible)return;
	UpdateProperty();
	if (!m_pShader) {
		m_pShader = new CSFShader();
		m_pShader->Build();
	}

	context.pResource->GL()->DisableCullFace();
	m_pShader->Use();
	m_pShader->SetModel(GetMatrix());
	m_pShader->SetPBRResource(context.pResource->GetPBR());
	m_pShader->SetCameraBuffer(context.pResource->GetCameraBuffer());
	m_pShader->SetLightBuffer(context.pResource->GetLightBuffer());
	m_pShader->SetMaterialBuffer(m_gpu.pMaterialBuffer.get());
	m_pShader->SetNodeBuffer(m_gpu.pNodeBuffer.get());
	m_pShader->SetVertexBuffer(m_gpu.pMeshBuffer[0]->pVertex.get());
	m_pShader->BindIndexBuffer(m_gpu.pMeshBuffer[0]->pIndex.get());

	for (const auto& solid : m_solids) {
		if (m_ui.visibleGeomIndex != -1 && solid.geomIndex != m_ui.visibleGeomIndex) continue;
		Vector2i value = Vector2i(solid.matrixIndex, solid.materialIndex);
		m_pShader->SetNode(value);
		m_pShader->Draw(m_solidDraw[solid.geomIndex][solid.drawIndex]);
		if (m_ui.showWire) {
			m_pShader->SetNode(Vector2i(solid.matrixIndex, 0));
			m_pShader->Draw(m_wireDraw[solid.geomIndex][solid.drawIndex]);
		}
	}
}

void CSFRenderNode::ShowUI(UIContext& ui)
{
	ImGui::Checkbox("Visible", &m_ui.visible);
	ImGui::Checkbox("Visible Wire", &m_ui.showWire);
	ImGui::SliderInt("VisibleIndex", &m_ui.visibleGeomIndex, -1, m_solidDraw.size());
	ImGui::Text("Parts Num %d\n Geometry Num %d\n", m_solids.size(), m_solidDraw.size());
	ImGui::Text("Triangle Num %d\n Line Num %d\n", m_property.triangleNum, m_property.lineNum);

}
}
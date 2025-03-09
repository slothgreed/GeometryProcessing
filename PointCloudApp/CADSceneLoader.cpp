#include "CADSceneLoader.h"
#define CSF_IMPLEMENTATION
#include "CSFNode.h"
#include "Utility.h"
#include <fileformats/cadscenefile.h>
namespace KI
{
CADSceneLoader::CADSceneLoader()
{
}

CADSceneLoader::~CADSceneLoader()
{

}

RenderNode* CADSceneLoader::Load(const String& fileName)
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
        materials[i].specular = ColorUtility::CreateRandom4() * 0.05f;
        materials[i].ambient = materials[i].diffuse * 0.05f;
        materials[i].emissive = materials[i].diffuse * 0.05f;
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
            // solid‚ÌŒã‚Éwire—p‚ÌIndexBuffer‚ª\’z‚³‚ê‚é‚Ì‚Å‚±‚±‚Å‰ÂŽZ‚·‚é
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

        // wire•ª‚ª‘«‚³‚ê‚Ä‚¢‚È‚¢‚Ì‚Å‚±‚±‚Å‰ÁŽZ‚·‚éB
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
}
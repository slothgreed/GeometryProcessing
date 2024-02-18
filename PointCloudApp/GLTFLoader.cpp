#include "GLTFLoader.h"
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/Deserialize.h>
 //Replace this with <filesystem> (and use std::filesystem rather than
 //std::experimental::filesystem) if your toolchain fully supports C++17
#include <filesystem>
#include <fstream>
#include <sstream>

#include "TextureLoader.h"
#include "FileUtility.h"
using namespace Microsoft::glTF;

// The glTF SDK is decoupled from all file I/O by the IStreamReader (and IStreamWriter)
// interface(s) and the C++ stream-based I/O library. This allows the glTF SDK to be used in
// sandboxed environments, such as WebAssembly modules and UWP apps, where any file I/O code
// must be platform or use-case specific.
class StreamReader : public IStreamReader
{
public:
    StreamReader(std::filesystem::path pathBase) : m_pathBase(std::move(pathBase))
    {
        assert(m_pathBase.has_root_path());
    }

    // Resolves the relative URIs of any external resources declared in the glTF manifest
    KI::Shared<std::istream> GetInputStream(const KI::String& filename) const override
    {
        // In order to construct a valid stream:
        // 1. The filename argument will be encoded as UTF-8 so use filesystem::u8path to
        //    correctly construct a path instance.
        // 2. Generate an absolute path by concatenating m_pathBase with the specified filename
        //    path. The filesystem::operator/ uses the platform's preferred directory separator
        //    if appropriate.
        // 3. Always open the file stream in binary mode. The glTF SDK will handle any text
        //    encoding issues for us.
        auto streamPath = m_pathBase / std::filesystem::u8path(filename);
        auto stream = std::make_shared<std::ifstream>(streamPath, std::ios_base::binary);

        // Check if the stream has no errors and is ready for I/O operations
        if (!stream || !(*stream)) {
            throw std::runtime_error("Unable to create a valid input stream for uri: " + filename);
        }

        return stream;
    }

private:
    std::filesystem::path m_pathBase;
};

namespace KI
{
class GLTFDocument::Impl
{
public:
    const Microsoft::glTF::Document& GetDocument() { return m_document; }
    void SetDocument(Document&& document) { m_document = std::move(document); }
private:
    Document m_document;
};

GLTFDocument::GLTFDocument()
    : m_pImpl(new GLTFDocument::Impl())
{
}


GLTFDocument::~GLTFDocument()
{
    delete m_pImpl;
    m_pImpl = nullptr;
}

const Microsoft::glTF::Document& GLTFDocument::GetDocument() const
{
    return m_pImpl->GetDocument();
}


void GLTFDocument::SetDocument(Microsoft::glTF::Document&& document)
{
    m_pImpl->SetDocument(std::move(document));
}

RenderNode* GLTFLoader::Load(const String& name)
{
    
    auto dirPath = KI::FileUtility::GetDirectoryPath(name);
    auto streamReader = std::make_unique<StreamReader>(dirPath);

    auto gltfStream = streamReader->GetInputStream(name);
    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(std::move(streamReader));

    std::stringstream manifestStream;

    manifestStream << gltfStream->rdbuf();

    Document document = Deserialize(manifestStream.str());

    auto meshes = LoadMesh(gltfResourceReader.get(), &document);
    auto textures = LoadTexture(dirPath, &document);
    auto materials = LoadMaterial(&document, textures);

    auto pNode = new GLTFScene(name);
    pNode->SetMesh(std::move(meshes));
    pNode->SetTexture(std::move(textures));
    pNode->SetMaterial(std::move(materials));
    return pNode;
}

MESH_TYPE ConvertMeshType(MeshMode mode)
{
    return MESH_TYPE(mode);
}

Vector<GLTFMesh> GLTFLoader::LoadMesh(const Microsoft::glTF::GLTFResourceReader* pResource, const Microsoft::glTF::Document* pDocument)
{
    Vector<GLTFMesh> meshes(pDocument->meshes.Size());

    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 texcoord;
        Vector4 tangent;
    };

    Vector<unsigned short> indexBuffer;
    Vector<Vertex> vertexBuffer;
    size_t vertexOffset = 0;
    for (size_t i = 0; i < meshes.size(); i++) {
        const auto& gltfMesh = pDocument->meshes.Get(i);
        Vector<GLTFPrimitive> primitives(gltfMesh.primitives.size());
        int drawOffset = 0;
        for (size_t j = 0; j < primitives.size(); j++) {
            vertexOffset = vertexBuffer.size();
            const auto& indices = pDocument->accessors.Get(gltfMesh.primitives[j].indicesAccessorId);
            primitives[j].meshType = ConvertMeshType(gltfMesh.primitives[j].mode);
            primitives[j].materialIndex = StringToInt(gltfMesh.primitives[j].materialId);
            primitives[j].drawNum = indices.count;
            primitives[j].drawOffset = drawOffset * sizeof(unsigned short);
            primitives[j].baseVertex = vertexBuffer.size();
            drawOffset += indices.count;
            if (indices.componentType == DATA_USHORT) {
                const auto& binary = pResource->ReadBinaryData<unsigned short>(*pDocument, indices);
                indexBuffer.insert(
                    indexBuffer.end(),
                    std::make_move_iterator(binary.begin()),
                    std::make_move_iterator(binary.end()));
            } else {
                assert(0);
            }

            if (gltfMesh.primitives[j].HasAttribute("POSITION")) {
                const auto& position = pDocument->accessors.Get(gltfMesh.primitives[j].attributes.find("POSITION")->second);
                if (position.componentType == GL_FLOAT &&
                    position.type == AccessorType::TYPE_VEC3) {
                    const auto& binary = pResource->ReadBinaryData<float>(*pDocument, position);
                    vertexBuffer.resize(vertexBuffer.size() + binary.size() / 3);
                    for (size_t k = 0; 3 * k < binary.size(); k++) {
                        vertexBuffer[k + vertexOffset].position = Vector3(binary[3 * k], binary[3 * k + 1], binary[3 * k + 2]);
                    }
                } else {
                    assert(0);
                }
            }

            if (gltfMesh.primitives[j].HasAttribute("POSITION")) {
                const auto& normal = pDocument->accessors.Get(gltfMesh.primitives[j].attributes.find("NORMAL")->second);
                if (normal.componentType == GL_FLOAT &&
                    normal.type == AccessorType::TYPE_VEC3) {
                    const auto& binary = pResource->ReadBinaryData<float>(*pDocument, normal);
                    for (size_t k = 0; 3 * k < binary.size(); k++) { vertexBuffer[k + vertexOffset].normal = Vector3(binary[3 * k], binary[3 * k + 1], binary[3 * k + 2]); }
                } else {
                    assert(0);
                }
            }
            if (gltfMesh.primitives[j].HasAttribute("TEXCOORD_0")) {
                const auto& texCoord = pDocument->accessors.Get(gltfMesh.primitives[j].attributes.find("TEXCOORD_0")->second);
                if (texCoord.componentType == GL_FLOAT &&
                    texCoord.type == AccessorType::TYPE_VEC2) {
                    const auto& binary = pResource->ReadBinaryData<float>(*pDocument, texCoord);
                    for (size_t k = 0; 2 * k < binary.size(); k++) { vertexBuffer[k + vertexOffset].texcoord = Vector2(binary[2 * k], binary[2 * k + 1]); }
                } else {
                    assert(0);
                }
            }
            if (gltfMesh.primitives[j].HasAttribute("TANGENT")) {
                const auto& tangent = pDocument->accessors.Get(gltfMesh.primitives[j].attributes.find("TANGENT")->second);
                if (tangent.componentType == GL_FLOAT &&
                    tangent.type == AccessorType::TYPE_VEC4) {
                    const auto& binary = pResource->ReadBinaryData<float>(*pDocument, tangent);
                    for (size_t k = 0; 4 * k < binary.size(); k++) { vertexBuffer[k + vertexOffset].tangent = Vector4(binary[4 * k], binary[4 * k + 1], binary[4 * k + 2], binary[4 * k + 3]); }
                } else {
                    assert(0);
                }
            }
        }

        auto pIndexBuffer = std::make_unique<GLBuffer>();
        pIndexBuffer->Create(indexBuffer);

        auto pVertexBuffer = std::make_unique<GLBuffer>();
        pVertexBuffer->Create(vertexBuffer.size(), sizeof(Vertex));
        pVertexBuffer->BufferSubData(0, vertexBuffer.size(), sizeof(Vertex), vertexBuffer.data());
        VertexFormats formats(4);

        formats[0].name = "POSITION";
        formats[0].location = 0;
        formats[0].componentSize = 3;
        formats[0].type = DATA_FLOAT;
        formats[0].offset = 0;


        formats[1].name = "NORMAL";
        formats[1].location = 1;
        formats[1].componentSize = 3;
        formats[1].type = DATA_FLOAT;
        formats[1].offset = offsetof(Vertex, normal);

        formats[2].name = "TEXCOORD_0";
        formats[2].location = 2;
        formats[2].componentSize = 2;
        formats[2].type = DATA_FLOAT;
        formats[2].offset = offsetof(Vertex, texcoord);

        formats[3].name = "TANGENT";
        formats[3].location = 3;
        formats[3].componentSize = 4;
        formats[3].type = DATA_FLOAT;
        formats[3].offset = offsetof(Vertex, tangent);

        meshes[i].SetVertexFormat(std::move(formats));
        meshes[i].SetVertexBuffer(std::move(pVertexBuffer));
        meshes[i].SetIndexBuffer(std::move(pIndexBuffer));
        meshes[i].SetPrimitives(std::move(primitives));
    }

    return meshes;
}
Vector<GLTFMaterial> GLTFLoader::LoadMaterial(const Microsoft::glTF::Document* pDocument, const Vector<Shared<Texture>>& textures)
{
    Vector<GLTFMaterial> materials(pDocument->materials.Size());
    for (size_t i = 0; i < pDocument->materials.Size(); i++) {
        const auto& material = pDocument->materials.Get(i);
        materials[i].pbr.baseColor = Vector4(
            material.metallicRoughness.baseColorFactor.r,
            material.metallicRoughness.baseColorFactor.g,
            material.metallicRoughness.baseColorFactor.b,
            material.metallicRoughness.baseColorFactor.a);
        materials[i].pbr.metalic = material.metallicRoughness.metallicFactor;
        if (textures.size() != 0) {
            materials[i].pbr.baseTexture = textures[StringToInt(material.metallicRoughness.baseColorTexture.textureId)];
            materials[i].normalTexture = textures[StringToInt(material.normalTexture.textureId)];
            materials[i].pbr.roughnessTexture = textures[StringToInt(material.metallicRoughness.metallicRoughnessTexture.textureId)];
        }
        materials[i].normalScale = material.normalTexture.scale;
    }

    return materials;
}

Vector<Shared<Texture>> GLTFLoader::LoadTexture(const String& directory, const Microsoft::glTF::Document* pDocument)
{
    Vector<Shared<Texture>> textures(pDocument->images.Size());
    for (size_t i = 0; i < pDocument->images.Size(); i++) {
        const auto& image = pDocument->images.Get(i);
        textures[i] = Shared<Texture>(TextureLoader::Load(directory + image.uri));
    }

    return textures;
}

}
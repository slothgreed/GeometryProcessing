#include "GLTFLoader.h"
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/Deserialize.h>
 //Replace this with <filesystem> (and use std::filesystem rather than
 //std::experimental::filesystem) if your toolchain fully supports C++17
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Utility.h"
#include "TextureLoader.h"
#include "FileUtility.h"
#include "Utility.h"
#include "GLTFScene.h"
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


Matrix4x4 Convert(const Matrix4& matrix)
{
    return Matrix4x4(
        matrix.values[0], matrix.values[1], matrix.values[2], matrix.values[3],
        matrix.values[4], matrix.values[5], matrix.values[6], matrix.values[7],
        matrix.values[8], matrix.values[9], matrix.values[10], matrix.values[11],
        matrix.values[12], matrix.values[13], matrix.values[14], matrix.values[15]);
}

Vector3 Convert(const Microsoft::glTF::Vector3& value)
{
    return Vector3(value.x, value.y, value.z);
}


Matrix4x4 Convert(const Microsoft::glTF::Quaternion& value)
{
    return glm::toMat4(glm::qua(value.x, value.y, value.z, value.w));
}

int ConvertIndex(const std::string& str)
{
    if (str.empty()) { return -1; }
    return StringToInt(str);
}


std::vector<int> ConvertIndex(const std::vector<std::string>& str)
{
    std::vector<int> intValue;

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i].empty()) { continue; }
        intValue.push_back(StringToInt(str[i]));
    }

    return intValue;
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

    Vector<MeshBuffer> meshBuffer(1);
    Unique<GLBuffer> matrixBuffer;
    BDB bdb;
    auto meshes = LoadMesh(gltfResourceReader.get(), &document, meshBuffer[0], bdb);
    auto textures = LoadTexture(dirPath, &document);
    auto materials = LoadMaterial(&document, textures);
    auto root = LoadNode(&document);
    auto animation = LoadAnimation(gltfResourceReader.get(), &document);
    auto skins = LoadSkin(&document, gltfResourceReader.get());
    auto pScene = new GLTFScene(name);

    pScene->SetBoundBox(bdb);
    pScene->SetMeshBuffer(std::move(meshBuffer));
    pScene->SetNode(std::move(root));
    pScene->SetSkin(std::move(skins));
    pScene->SetRoot(ConvertIndex(document.scenes[0].nodes));
    pScene->SetMesh(std::move(meshes));
    pScene->SetTexture(std::move(textures));
    pScene->SetMaterial(std::move(materials));
    pScene->SetAnimation(std::move(animation));
    pScene->Initialize();
    return pScene;
}

Vector<GLTFNode> GLTFLoader::LoadNode(const Microsoft::glTF::Document* pDocument)
{
    Vector<GLTFNode> nodes(pDocument->nodes.Size());
    for (size_t i = 0; i < pDocument->nodes.Size(); i++) {
        nodes[i].SetIndex(ConvertIndex(pDocument->nodes[i].id));
        nodes[i].SetRotate(Convert(pDocument->nodes[i].rotation));
        nodes[i].SetScale(Convert(pDocument->nodes[i].scale));
        nodes[i].SetTranslate(Convert(pDocument->nodes[i].translation));
        nodes[i].SetBaseMatrix(Convert(pDocument->nodes[i].matrix));
        nodes[i].SetSkinId(ConvertIndex(pDocument->nodes[i].skinId));
        nodes[i].SetMeshId(ConvertIndex(pDocument->nodes[i].meshId));
        nodes[i].SetChild(ConvertIndex(pDocument->nodes[i].children));
    }

    return nodes;
}


Vector<GLTFSkin> GLTFLoader::LoadSkin(const Microsoft::glTF::Document* pDocument, const Microsoft::glTF::GLTFResourceReader* pResource)
{
    Vector<GLTFSkin> skins;
    for (size_t i = 0; i < pDocument->skins.Size(); i++) {
        const auto& gltfSkin = pDocument->skins[i];
        GLTFSkin skin;
        skin.SetName(gltfSkin.name);
        skin.SetRootSkeleton(StringToInt(gltfSkin.skeletonId));
        Vector<int> joints(gltfSkin.jointIds.size());
        for (size_t i = 0; i < gltfSkin.jointIds.size(); i++) {
            joints[i] = StringToInt(gltfSkin.jointIds[i]);
        }
        skin.SetJointNodeIndex(std::move(joints));

        int inverseBindMatrices =  StringToInt(gltfSkin.inverseBindMatricesAccessorId);
        if (inverseBindMatrices > -1) {
            const auto& accessor = pDocument->accessors[inverseBindMatrices];
            const auto& bufferView = pDocument->bufferViews[StringToInt(accessor.bufferViewId)];
            auto matrix = pResource->ReadBinaryData<glm::mat4x4>(*pDocument, bufferView);
            skin.SetInverseBindMatrix(std::move(matrix));
        }
        skins.push_back(std::move(skin));
    }

    return skins;
}
PRIMITIVE_TYPE ConvertPrimitiveType(MeshMode mode)
{
    return PRIMITIVE_TYPE(mode);
}

GLTFAnimation::Channel::Path ConvertAnimationPathType(TargetPath path)
{
    if (path == TARGET_ROTATION) {
        return GLTFAnimation::Channel::Path::Rotate;
    } else if (path == TARGET_SCALE) {
        return GLTFAnimation::Channel::Path::Scale;
    } else if (path == TARGET_TRANSLATION) {
        return GLTFAnimation::Channel::Path::Translate;
    } else if (path == TARGET_WEIGHTS) {
        return GLTFAnimation::Channel::Path::Weight;
    } else {
        assert(0);
        return GLTFAnimation::Channel::Path::Translate;
    }
}


Vector<GLTFAnimation> GLTFLoader::LoadAnimation(const Microsoft::glTF::GLTFResourceReader* pResource, const Microsoft::glTF::Document* pDocument)
{
    Vector<GLTFAnimation> animations(pDocument->animations.Size());
    for (size_t i = 0; i < pDocument->animations.Size(); i++) {
        const auto& animation = pDocument->animations.Get(i);
        Vector<GLTFAnimation::Channel> channels(animation.channels.Size());
        for (size_t j = 0; j < animation.channels.Size(); j++) {
            const auto& gltfChannel = animation.channels.Get(j);
            channels[j].sampler = StringToInt(gltfChannel.samplerId);
            channels[j].node = StringToInt(gltfChannel.target.nodeId);
            channels[j].path = ConvertAnimationPathType(gltfChannel.target.path);
        }
        Vector<GLTFAnimation::Sampler> samplers(animation.samplers.Size());
        for (size_t j = 0; j < animation.samplers.Size(); j++) {
            auto& sampler = samplers[j];
            const auto& gltfSampler = animation.samplers.Get(j);
            const auto& inputAccessor = pDocument->accessors.Get(StringToInt(gltfSampler.inputAccessorId));
            const auto& outputAccessor = pDocument->accessors.Get(StringToInt(gltfSampler.outputAccessorId));
            const auto& output = pResource->ReadFloatData(*pDocument, outputAccessor);
            sampler.timer = pResource->ReadFloatData(*pDocument, inputAccessor);
            if (outputAccessor.type == AccessorType::TYPE_VEC3) {
                sampler.transform.resize(output.size() / 3);
                for (size_t k = 0; k < sampler.transform.size(); k++) {
                    sampler.transform[k].x = output[3 * k];
                    sampler.transform[k].y = output[3 * k + 1];
                    sampler.transform[k].z = output[3 * k + 2];
                    sampler.transform[k].w = 1.0;
                }
            } else if(outputAccessor.type == AccessorType::TYPE_VEC4) {
                sampler.transform.resize(output.size() / 4);
                for (size_t k = 0; k < sampler.transform.size(); k++) {
                    sampler.transform[k].x = output[4 * k];
                    sampler.transform[k].y = output[4 * k + 1];
                    sampler.transform[k].z = output[4 * k + 2];
                    sampler.transform[k].w = output[4 * k + 3];
                }
            } else {
                assert(0);
            }
        }
        animations[i].SetChannel(std::move(channels));
        animations[i].SetSampler(std::move(samplers));
    }

    return animations;
}
Vector<GLTFMesh> GLTFLoader::LoadMesh(const Microsoft::glTF::GLTFResourceReader* pResource, const Microsoft::glTF::Document* pDocument, MeshBuffer& pMeshBuffer, BDB& bdb)
{
    Vector<GLTFMesh> meshes(pDocument->meshes.Size());

    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 texcoord;
        Vector4 tangent;
        Vector4 joint;
        Vector4 weight;
    };

    Vector<unsigned short> indexBuffer;
    Vector<Vertex> vertexBuffer;
    size_t vertexOffset = 0;
    bool hasPosition = false;
    bool hasNormal = false;
    bool hasTexcoord = false;
    bool hasTangent = false;
    bool hasJoint = false;
    bool hasWeight = false;
    int drawOffset = 0;
    for (size_t i = 0; i < meshes.size(); i++) {
        const auto& gltfMesh = pDocument->meshes.Get(i);
        Vector<GLTFPrimitive> primitives(gltfMesh.primitives.size());
        for (size_t j = 0; j < primitives.size(); j++) {
            vertexOffset = vertexBuffer.size();
            const auto& indices = pDocument->accessors.Get(gltfMesh.primitives[j].indicesAccessorId);
            primitives[j].primitiveType = ConvertPrimitiveType(gltfMesh.primitives[j].mode);
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
                if (position.componentType == GL_FLOAT && position.type == AccessorType::TYPE_VEC3) {
                    const auto& binary = pResource->ReadBinaryData<float>(*pDocument, position);
                    vertexBuffer.resize(vertexBuffer.size() + binary.size() / 3);
                    for (size_t k = 0; 3 * k < binary.size(); k++) {
                        vertexBuffer[k + vertexOffset].position = Vector3(binary[3 * k], binary[3 * k + 1], binary[3 * k + 2]);
                        bdb.Add(vertexBuffer[k + vertexOffset].position);
                    }
                } else {
                    assert(0);
                }
                hasPosition = true;
            }

            if (gltfMesh.primitives[j].HasAttribute("NORMAL")) {
                const auto& normal = pDocument->accessors.Get(gltfMesh.primitives[j].attributes.find("NORMAL")->second);
                if (normal.componentType == GL_FLOAT && normal.type == AccessorType::TYPE_VEC3) {
                    const auto& binary = pResource->ReadBinaryData<float>(*pDocument, normal);
                    for (size_t k = 0; 3 * k < binary.size(); k++) { vertexBuffer[k + vertexOffset].normal = Vector3(binary[3 * k], binary[3 * k + 1], binary[3 * k + 2]); }
                } else {
                    assert(0);
                }

                hasNormal = true;
            }
            if (gltfMesh.primitives[j].HasAttribute("TEXCOORD_0")) {
                const auto& texCoord = pDocument->accessors.Get(gltfMesh.primitives[j].attributes.find("TEXCOORD_0")->second);
                if (texCoord.componentType == GL_FLOAT && texCoord.type == AccessorType::TYPE_VEC2) {
                    const auto& binary = pResource->ReadBinaryData<float>(*pDocument, texCoord);
                    for (size_t k = 0; 2 * k < binary.size(); k++) { vertexBuffer[k + vertexOffset].texcoord = Vector2(binary[2 * k], binary[2 * k + 1]); }
                } else {
                    assert(0);
                }
                hasTexcoord = true;
            }
            if (gltfMesh.primitives[j].HasAttribute("TANGENT")) {
                const auto& tangent = pDocument->accessors.Get(gltfMesh.primitives[j].attributes.find("TANGENT")->second);
                if (tangent.componentType == GL_FLOAT && tangent.type == AccessorType::TYPE_VEC4) {
                    const auto& binary = pResource->ReadBinaryData<float>(*pDocument, tangent);
                    for (size_t k = 0; 4 * k < binary.size(); k++) { vertexBuffer[k + vertexOffset].tangent = Vector4(binary[4 * k], binary[4 * k + 1], binary[4 * k + 2], binary[4 * k + 3]); }
                } else {
                    assert(0);
                }
                hasTangent = true;
            }

            if (gltfMesh.primitives[j].HasAttribute("JOINTS_0")) {
                const auto& joint = pDocument->accessors.Get(gltfMesh.primitives[j].attributes.find("JOINTS_0")->second);
                if (joint.componentType == COMPONENT_UNSIGNED_SHORT && joint.type == AccessorType::TYPE_VEC4) {
                    const auto& binary = pResource->ReadBinaryData<unsigned short>(*pDocument, joint);
                    for (size_t k = 0; 4 * k < binary.size(); k++) { vertexBuffer[k + vertexOffset].joint = Vector4(binary[4 * k], binary[4 * k + 1], binary[4 * k + 2], binary[4 * k + 3]); }
                } else {
                    assert(0);
                }
                hasJoint = true;
            }

            if (gltfMesh.primitives[j].HasAttribute("WEIGHTS_0")) {
                const auto& weight = pDocument->accessors.Get(gltfMesh.primitives[j].attributes.find("WEIGHTS_0")->second);
                if (weight.componentType == GL_FLOAT && weight.type == AccessorType::TYPE_VEC4) {
                    const auto& binary = pResource->ReadBinaryData<float>(*pDocument, weight);
                    for (size_t k = 0; 4 * k < binary.size(); k++) { vertexBuffer[k + vertexOffset].weight = Vector4(binary[4 * k], binary[4 * k + 1], binary[4 * k + 2], binary[4 * k + 3]); }
                } else {
                    assert(0);
                }
                hasWeight = true;
            }
        }
        meshes[i].SetPrimitives(std::move(primitives));
        meshes[i].SetName(gltfMesh.name);
        meshes[i].SetMeshBufferIndex(0);
    }

    auto pIndexBuffer = std::make_unique<GLBuffer>();
    pIndexBuffer->Create(indexBuffer);

    auto pVertexBuffer = std::make_unique<GLBuffer>();
    pVertexBuffer->Create<Vertex>(vertexBuffer);

    VertexFormats formats;
    if (hasPosition) {
        VertexFormat format;
        format.name = "POSITION";
        format.location = 0;
        format.componentSize = 3;
        format.type = DATA_FLOAT;
        format.offset = 0;
        formats.push_back(format);
    }

    if (hasNormal) {
        VertexFormat format;
        format.name = "NORMAL";
        format.location = 1;
        format.componentSize = 3;
        format.type = DATA_FLOAT;
        format.offset = offsetof(Vertex, normal);
        formats.push_back(format);
    }

    if (hasTexcoord) {
        VertexFormat format;
        format.name = "TEXCOORD_0";
        format.location = 2;
        format.componentSize = 2;
        format.type = DATA_FLOAT;
        format.offset = offsetof(Vertex, texcoord);
        formats.push_back(format);
    }

    if (hasTangent) {
        VertexFormat format;
        format.name = "TANGENT";
        format.location = 3;
        format.componentSize = 4;
        format.type = DATA_FLOAT;
        format.offset = offsetof(Vertex, tangent);
        formats.push_back(format);
    }


    if (hasJoint) {
        VertexFormat format;
        format.name = "JOINT";
        format.location = 4;
        format.componentSize = 4;
        format.type = DATA_FLOAT;
        format.offset = offsetof(Vertex, joint);
        formats.push_back(format);
    }

    if (hasWeight) {
        VertexFormat format;
        format.name = "WEIGHT";
        format.location = 5;
        format.componentSize = 4;
        format.type = DATA_FLOAT;
        format.offset = offsetof(Vertex, weight);
        formats.push_back(format);
    }


    pMeshBuffer.format = formats;
    pMeshBuffer.pVertex = std::move(pVertexBuffer);
    pMeshBuffer.pIndex = std::move(pIndexBuffer);

    return meshes;
}

Vector<GLTFMaterial> GLTFLoader::LoadMaterial(const Microsoft::glTF::Document* pDocument, const Vector<Shared<Texture>>& textures)
{
    Vector<GLTFMaterial> materials(pDocument->materials.Size());
    for (size_t i = 0; i < pDocument->materials.Size(); i++) {
        const auto& material = pDocument->materials.Get(i);
        materials[i].baseColor = Vector4(
            material.metallicRoughness.baseColorFactor.r,
            material.metallicRoughness.baseColorFactor.g,
            material.metallicRoughness.baseColorFactor.b,
            material.metallicRoughness.baseColorFactor.a);
        materials[i].metallic = material.metallicRoughness.metallicFactor;
        materials[i].roughness = material.metallicRoughness.roughnessFactor;
        materials[i].baseTexture = ConvertIndex(material.metallicRoughness.baseColorTexture.textureId);
        materials[i].normalTexture = ConvertIndex(material.normalTexture.textureId);
        materials[i].metalRoughnessTexture = ConvertIndex(material.metallicRoughness.metallicRoughnessTexture.textureId);
        materials[i].emissiveTexture = ConvertIndex(material.emissiveTexture.textureId);
        materials[i].occlusionTexture = ConvertIndex(material.occlusionTexture.textureId);
        materials[i].normalScale = material.normalTexture.scale;
        materials[i].emissiveColor = Vector4(material.emissiveFactor.r, material.emissiveFactor.g, material.emissiveFactor.b, 1.0);
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
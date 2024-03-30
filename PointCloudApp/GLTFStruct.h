#ifndef KI_GLTF_STRUCT_H
#define KI_GLTF_STRUCT_H
#include "RenderNode.h"
namespace Microsoft
{
	namespace glTF
	{
		struct Mesh;
		struct MeshPrimitive;
		struct Accessor;
	}
}

namespace KI
{
class GLTFDocument;

class GLTFNode
{
public:
	GLTFNode()
		: m_index()
		, m_meshId(-1)
		, m_localMatrix(Matrix4x4(1.0f))
		, m_matrix(Matrix4x4(1.0f)){ };
	~GLTFNode() {};
	void SetIndex(int id) { m_index = id; }
	void SetLocalMatrix(const Matrix4x4& local) { m_localMatrix = local; }
	const Matrix4x4& GetLocalMatrix() const { return m_localMatrix; }
	void SetMatrix(const Matrix4x4& matrix) { m_matrix = matrix; }
	void SetMeshId(int id) { m_meshId = id; }
	int GetMeshId() const { return m_meshId; }
	int GetIndex() const { return m_index; }
	const Matrix4x4& GetMatrix() const { return m_matrix; }
	const Vector<int>& GetChild() const { return m_child; }
	void SetChild(Vector<int>&& child) { m_child = child; }

protected:
	Matrix4x4 m_localMatrix;
	Matrix4x4 m_matrix;
	int m_meshId;
	int m_index;
	Vector<int> m_child;
};

// SSBOÇ≈ShaderÇ≈égÇ¡ÇƒÇÈÅB
struct GLTFMaterial
{
	static const int NO_TEXTURE = -1;
	enum class AlphaMode
	{
		Unknown,
		Opaque,
		Blend,
		Mask,
	};

	Vector4 baseColor;
	int baseTexture;
	float metalic;
	int roughnessTexture;
	int normalTexture;
	float normalScale;
	float alphaCuttoff;
	int alphaMode;
	bool doubleSided;
	float padding[9];  //36
};

struct GLTFPrimitive
{
	GLTFPrimitive()
		: materialIndex(0)
		, meshType(MESH_POINTS)
		, drawNum(0)
		, baseVertex(0)
		, drawOffset(0)
	{
	}
	int materialIndex;
	MESH_TYPE meshType;
	int drawNum;
	size_t drawOffset;
	size_t baseVertex;
};

struct MeshBuffer
{
	VertexFormats format;
	Unique<GLBuffer> pVertex;
	Unique<GLBuffer> pIndex;
};

class GLTFMesh
{
public:
	GLTFMesh()
		:m_bufferIndex(-1)
	{
	}
	~GLTFMesh() {};

	void SetName(const String& name) { m_name = name; }
	void SetMeshBufferIndex(int index) { m_bufferIndex = index; }
	void SetPrimitives(Vector<GLTFPrimitive>&& value) { m_primitives = std::move(value); }

	int GetBufferIndex() const { return m_bufferIndex; }
	const Vector<GLTFPrimitive>& GetPrimitives() const { return m_primitives; }
private:
	String m_name;
	int m_bufferIndex;
	Vector<GLTFPrimitive> m_primitives;
};



class GLTFAnimation
{
public:

	GLTFAnimation() {};
	~GLTFAnimation() {};

	struct Sampler
	{
		std::vector<float> timer;
		std::vector<Vector4> transform;
	};

	struct Channel
	{
		enum Path
		{
			Translate,
			Scale,
			Rotate,
			Weight
		};


		int sampler;
		int node;
		Path path;
	};


	void SetSampler(Vector<Sampler>&& sampler) { m_samplers = std::move(sampler); }
	void SetChannel(Vector<Channel>&& channel) { m_chennels = std::move(channel); }

	const Vector<Sampler>& GetSamplers() const { return m_samplers; }
	const Vector<Channel>& GetChannels() const { return m_chennels; }
private:
	Vector<Sampler> m_samplers;
	Vector<Channel> m_chennels;
};


class GLTFShader;
class GLTFScene : public RenderNode
{
public:
	GLTFScene(const String& name) 
		: RenderNode(name)
		, m_pShader(nullptr)
		, m_pMaterials(nullptr) {};
	~GLTFScene() {};

	void SetNode(Vector<GLTFNode>&& node) { m_nodes = std::move(node); };
	void SetRoot(Vector<int>&& root) { m_roots = std::move(root); }
	void SetMaterial(Vector<GLTFMaterial>&& value) { m_material = std::move(value); }
	void SetTexture(Vector<Shared<Texture>>&& value) { m_texture = std::move(value); }
	void SetMeshBuffer(Vector<MeshBuffer>&& buffer) { m_meshBuffer = std::move(buffer); }
	void SetMesh(Vector<GLTFMesh>&& value) { m_meshes = std::move(value); }
	void SetAnimation(Vector<GLTFAnimation>&& animation) { m_animation = std::move(animation); }
	virtual void Draw(const Matrix4x4& proj, const Matrix4x4& view);
protected:
	virtual void UpdateData(float time);
private:
	void UpdateMatrix(int index, const Matrix4x4& mat);

	void CreateMaterialBuffer();
	GLTFShader* m_pShader;
	GLBuffer* m_pMaterials;
	Vector<int> m_roots;
	Vector<GLTFMaterial> m_material;
	Vector<Shared<Texture>> m_texture;
	Vector<MeshBuffer> m_meshBuffer;
	Vector<GLTFMesh> m_meshes;
	Vector<GLTFNode> m_nodes;
	Vector<GLTFAnimation> m_animation;
};
}
#endif KI_GLTF_STRUCT_H
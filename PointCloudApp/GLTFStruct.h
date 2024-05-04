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
class GLTFSkin
{
public:
	GLTFSkin()
		: m_rootSkeleton(0)
		, m_inverseBindMatrix(0){}
	~GLTFSkin() {};
	void SetName(const String& name) { m_name = name; }
	void SetRootSkeleton(int value) { m_rootSkeleton = value; }
	void SetInverseBindMatrix(Vector<Matrix4x4>&& matrix) { m_inverseBindMatrix = matrix; }
	void SetJointNodeIndex(Vector<int>&& node) { m_nodeIndex = std::move(node); }
	const Vector<int>& GetJointNodeIndex() const { return m_nodeIndex; }
private:
	String m_name;
	int m_rootSkeleton;
	Vector<Matrix4x4> m_inverseBindMatrix;
	Vector<int> m_nodeIndex;
};

class GLTFNode
{
public:
	struct GpuObject
	{
		Matrix4x4 matrix;
		Matrix4x4 localMatrix;
		int jointCount;
		float padding[31];
	};

	GLTFNode()
		: m_index(-1)
		, m_skinId(-1)
		, m_meshId(-1)
		, m_localMatrix(Matrix4x4(1.0f))
		, m_matrix(Matrix4x4(1.0f)){ };
	~GLTFNode() {};
	void SetIndex(int id) { m_index = id; }
	void SetLocalMatrix(const Matrix4x4& local) { m_localMatrix = local; }
	const Matrix4x4& GetLocalMatrix() const { return m_localMatrix; }
	void SetMatrix(const Matrix4x4& matrix) { m_matrix = matrix; }
	void SetMeshId(int id) { m_meshId = id; }
	void SetSkinId(int id) { m_skinId = id; }
	int GetSkinId() const { return m_skinId; }
	int GetMeshId() const { return m_meshId; }
	int GetIndex() const { return m_index; }
	const Matrix4x4& GetMatrix() const { return m_matrix; }
	const Vector<int>& GetChild() const { return m_child; }
	void SetChild(Vector<int>&& child) { m_child = child; }

	static Vector<GLTFNode::GpuObject> CreateGpuObject(const Vector<GLTFNode>& nodes, const Vector<GLTFSkin>& skins);
	static void UpdateMatrix(const Vector<int>& roots, Vector<GLTFNode>& nodes);
	static Matrix4x4 CreateMatrix(const Vector3& scale, const Matrix4x4& rotate, const Vector3& translate);

protected:
	static void UpdateMatrixRecursive(Vector<GLTFNode>& nodes, int index, const Matrix4x4& matrix);
	int m_skinId;
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

	Vector4 baseColor;	// 16
	int baseTexture;	// 20
	float metalic;		// 24
	int roughnessTexture;// 28
	int normalTexture;	// 32
	float normalScale;	// 36
	float alphaCuttoff;	// 40
	int alphaMode;		// 44
	int doubleSided;	// 48
	float padding[52];  // 256 
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

	static void Update(const Vector<GLTFAnimation>& animations, Vector<GLTFNode>& nodes, float time);
private:
	Vector<Sampler> m_samplers;
	Vector<Channel> m_chennels;
};



class GLTFShader;
class GLTFSceneMatrixUpdaterOnGpu;
class GLTFScene : public RenderNode
{
public:
	GLTFScene(const String& name) 
		: RenderNode(name)
		, m_pShader(nullptr)
		, m_pNodeBuffer(nullptr)
		, m_pMaterials(nullptr)
		, m_pMatrixGpuUpdater(nullptr){};
	~GLTFScene();

	void Initialize();
	void SetNode(Vector<GLTFNode>&& node) { m_nodes = std::move(node); };
	void SetRoot(Vector<int>&& root) { m_roots = std::move(root); }
	void SetMaterial(Vector<GLTFMaterial>&& value) { m_material = std::move(value); }
	void SetTexture(Vector<Shared<Texture>>&& value) { m_texture = std::move(value); }
	void SetMeshBuffer(Vector<MeshBuffer>&& buffer) { m_meshBuffer = std::move(buffer); }
	void SetSkin(Vector<GLTFSkin>&& value) { m_skins = std::move(value); }
	void SetMesh(Vector<GLTFMesh>&& value) { m_meshes = std::move(value); }
	void SetAnimation(Vector<GLTFAnimation>&& animation) { m_animation = std::move(animation); }
	virtual void Draw(const Matrix4x4& proj, const Matrix4x4& view);
protected:
	virtual void UpdateData(float time);
private:
	void UpdateMatrix();

	void CreateMaterialBuffer();
	GLTFShader* m_pShader;
	GLBuffer* m_pMaterials;
	GLBuffer* m_pNodeBuffer;
	Vector<MeshBuffer> m_meshBuffer;


	GLTFSceneMatrixUpdaterOnGpu* m_pMatrixGpuUpdater;

	Vector<int> m_roots;
	Vector<Shared<Texture>> m_texture;
	Vector<GLTFMaterial> m_material;
	Vector<GLTFNode> m_nodes;
	Vector<GLTFSkin> m_skins;
	Vector<GLTFMesh> m_meshes;
	Vector<GLTFAnimation> m_animation;

	struct GpuObject
	{
		Vector<GLTFNode::GpuObject> node;
	};

	GpuObject m_gpu;

};
}
#endif KI_GLTF_STRUCT_H
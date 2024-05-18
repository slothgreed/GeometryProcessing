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
class GLTFNode;
class GLTFDocument;
class GLTFSkin
{
public:
	GLTFSkin()
		: m_rootSkeleton(0)
		, m_inverseBindMatrix(){}
	~GLTFSkin() {};

	void SetName(const String& name) { m_name = name; }
	void SetRootSkeleton(int value) { m_rootSkeleton = value; }
	void SetInverseBindMatrix(Vector<Matrix4x4>&& matrix) { m_inverseBindMatrix = matrix; m_jointMatrix.resize(m_inverseBindMatrix.size(), mat4x4(1.0)); }
	void SetJointNodeIndex(Vector<int>&& node) { m_jointNodeIndex = std::move(node); }
	const Vector<int>& GetJointNodeIndex() const { return m_jointNodeIndex; }
	int GetRootSkeleton() const { return m_rootSkeleton; }
	struct GpuObject
	{
		// MemoryLayout
		// int skeleton;
		// int jointSize;
		// int jointNode[0];
		// mat4x4 inverseBindMatrix[0];
		// mat4x4 jointMatrix[0]
		// ・・・
		// int jointNode[n];
		// mat4x4 inverseBindMatrix[n];
		// mat4x4 jointMatrix[n];


		std::vector<float> data;
	};

	static GLTFSkin::GpuObject CreateGpuObject(const Vector<GLTFSkin>& skins);
	static void Update(Vector<GLTFSkin>& skins, const Vector<GLTFNode>& nodes);
private:
	static int CalcGpuMemorySize(const Vector<GLTFSkin>& skins);
	String m_name;
	int m_rootSkeleton;
	Vector<Matrix4x4> m_inverseBindMatrix;
	Vector<Matrix4x4> m_jointMatrix;
	Vector<int> m_jointNodeIndex;
};

class GLTFNode
{
public:
	struct GpuObject
	{
		Matrix4x4 matrix;	 // 16
		Vector4 scale;		 // 4
		Vector4 translate;	 // 4
		Matrix4x4 rotate;	 // 16
		Matrix4x4 localMatrix;// 16
		int skinId;			 // 1
		float padding[23];	 // 7
	};

	GLTFNode()
		: m_index(-1)
		, m_skinId(-1)
		, m_meshId(-1)
		, m_rotate(Matrix4x4(1.0f))
		, m_scale(Vector3(0.0f))
		, m_translate(Vector3(0.0f))
		, m_matrix(Matrix4x4(1.0f))
		, m_baseMatrix(Matrix4x4(1.0f))
		, m_localMatrix(Matrix4x4(1.0f)){ };
	~GLTFNode() {};
	void SetIndex(int id) { m_index = id; }
	void SetScale(const Vector3& local) { m_scale = local; m_localMatrix = CreateLocalMatrix(); }
	const Vector3& GetScale() const { return m_scale; }
	void SetRotate(const Matrix4x4& local) { m_rotate = local;  m_localMatrix = CreateLocalMatrix();}
	const Matrix4x4& GetRotate() const { return m_rotate; }
	void SetTranslate(const Vector3& local) { m_translate = local; m_localMatrix = CreateLocalMatrix(); }
	const Vector3& GetTranslate() const { return m_translate; }

	void SetMatrix(const Matrix4x4& matrix) { m_matrix = matrix; }
	void SetMeshId(int id) { m_meshId = id; }
	void SetSkinId(int id) { m_skinId = id; }
	int GetSkinId() const { return m_skinId; }
	int GetMeshId() const { return m_meshId; }
	int GetIndex() const { return m_index; }
	void SetBaseMatrix(const Matrix4x4& baseMatrix) { m_baseMatrix = baseMatrix; m_localMatrix = CreateLocalMatrix();
	}
	const Matrix4x4& GetBaseMatrix() const { return m_baseMatrix; }
	const Matrix4x4& GetMatrix() const { return m_matrix; }
	const Vector<int>& GetChild() const { return m_child; }
	void SetChild(Vector<int>&& child) { m_child = child; }

	static Vector<GLTFNode::GpuObject> CreateGpuObject(const Vector<GLTFNode>& nodes, const Vector<GLTFSkin>& skins);
	static void UpdateMatrix(const Vector<int>& roots, Vector<GLTFNode>& nodes);
	static Matrix4x4 CreateMatrix(const Vector3& scale, const Matrix4x4& rotate, const Vector3& translate);

	const Matrix4x4& GetLocalMatrix() const { return m_localMatrix; }
protected:
	Matrix4x4 CreateLocalMatrix() const;

	static void UpdateMatrixRecursive(Vector<GLTFNode>& nodes, int index, const Matrix4x4& matrix);
	int m_skinId;
	Vector3 m_scale;
	Vector3 m_translate;
	Matrix4x4 m_rotate;
	Matrix4x4 m_localMatrix;
	Matrix4x4 m_baseMatrix;
	Matrix4x4 m_matrix;
	int m_meshId;
	int m_index;
	Vector<int> m_child;
};

// SSBOでShaderで使ってる。
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

	// dataにはtimer,transform.x,transform.y,transform.zが入る
	// 最大サイズは63
	// SSBO同様256にしている。
	struct SamplerGpuObject
	{
		SamplerGpuObject()
		:size(0) {}
		float size;
		std::vector<float> data;
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

	typedef Channel ChannelGpuObject;


	void SetSampler(Vector<Sampler>&& sampler) { m_samplers = std::move(sampler); }
	void SetChannel(Vector<Channel>&& channel) { m_chennels = std::move(channel); }

	const Vector<Sampler>& GetSamplers() const { return m_samplers; }
	const Vector<Channel>& GetChannels() const { return m_chennels; }

	Vector<SamplerGpuObject> CreateSamplerGpuObject();
	Vector<ChannelGpuObject> CreateChannelGpuObject();
	static void Update(const Vector<GLTFAnimation>& animations, Vector<GLTFNode>& nodes, float time);
	
private:
	Vector<Sampler> m_samplers;
	Vector<Channel> m_chennels;
};

}
#endif KI_GLTF_STRUCT_H
#ifndef KI_GLTF_NODE_H
#define KI_GLTF_NODE_H
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

class GLTFNode : public RenderNode
{
public:
	GLTFNode(int index)
		: RenderNode(IntToString(index))
		, m_index(index) { };
	~GLTFNode() {};
	const Matrix4x4& GetMatrix(const GLTFDocument* pDocument);
	const Microsoft::glTF::Mesh* GetMesh(const GLTFDocument* pDocument);
	const Microsoft::glTF::Accessor* GetAccessor(const GLTFDocument* pDocument, const Microsoft::glTF::MeshPrimitive& primitive);
	int GetIndex() const { return m_index; }
	const Vector<Shared<GLTFNode>>& GetChild() const { return m_child; }
	void SetChild(Vector<Shared<GLTFNode>>&& child) { m_child = child; };
protected:
virtual void DrawData(const Matrix4x4& proj, const Matrix4x4& view) {}; private:
	Optional<Matrix4x4> m_matrix;
	int m_index;
	Vector<Shared<GLTFNode>> m_child;
};

struct GLTFMaterial
{
	struct PBRMetalic
	{
		Vector4 baseColor;
		Shared<Texture> baseTexture;
		float metalic;
		Shared<Texture> roughnessTexture; 
	};

	enum class AlphaMode
	{
		Unknown,
		Opaque,
		Blend,
		Mask,
	};

	PBRMetalic pbr;
	Shared<Texture> normalTexture;
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
	{
	}
	int materialIndex;
	MESH_TYPE meshType;
	int drawNum;
	size_t drawOffset;
	size_t baseVertex;
};

class GLTFMesh
{
public:
	GLTFMesh() {}
	~GLTFMesh() {};

	void SetVertexFormat(VertexFormats&& format) { m_formats = std::move(format); }
	void SetPrimitives(Vector<GLTFPrimitive>&& value) { m_primitives = std::move(value); }
	void SetVertexBuffer(Unique<GLBuffer>&& value) { m_vertexBuffer = std::move(value); }
	void SetIndexBuffer(Unique<GLBuffer>&& value) { m_indexBuffer = std::move(value); }

	const VertexFormats& GetFormats() const { return m_formats; }
	const Unique<GLBuffer>& GetVertexBuffer() const { return m_vertexBuffer; }
	const Unique<GLBuffer>& GetIndexBuffer() const { return m_indexBuffer; }
	const Vector<GLTFPrimitive>& GetPrimitives() const { return m_primitives; }
private:
	VertexFormats m_formats;
	Unique<GLBuffer> m_vertexBuffer;
	Unique<GLBuffer> m_indexBuffer;
	Vector<GLTFPrimitive> m_primitives;
};


class GLTFScene : public RenderNode
{
public:
	GLTFScene(const String& name) : RenderNode(name) {};
	~GLTFScene() {};

	void SetRoot(Unique<GLTFNode>&& node) { m_pRoot = std::move(node); };
	const GLTFNode* GetRoot() const { return m_pRoot.get(); }
	void SetMaterial(Vector<GLTFMaterial>&& value) { m_material = std::move(value); }
	void SetTexture(Vector<Shared<Texture>>&& value) { m_texture = std::move(value); }
	void SetMesh(Vector<GLTFMesh>&& value) { m_meshes = std::move(value); }
	GLTFNode* GetRoot() { return m_pRoot.get(); }
	virtual void Draw(const Matrix4x4& proj, const Matrix4x4& view);
private:
	Vector<GLTFMaterial> m_material;
	Vector<Shared<Texture>> m_texture;
	Vector<GLTFMesh> m_meshes;
	Unique<GLTFNode> m_pRoot;
};
}
#endif KI_GLTF_NODE_H
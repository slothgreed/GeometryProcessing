#ifndef KI_CSF_NODE_H
#define KI_CSF_NODE_H
#include "RenderNode.h"
namespace KI
{

struct CSFNode
{
    Matrix4x4 matrix;
    Matrix4x4 world;
    Matrix4x4 matrixIT;
    Matrix4x4 worldIT;
};


struct CSFMaterial
{
	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;
	Vector4 emissive;
	float padding[192];
};


struct CSFMeshBuffer
{
	CSFMeshBuffer() {}
	void Init() { pVertex = std::make_unique<GLBuffer>(); pIndex = std::make_unique<GLBuffer>(); }
	Unique<GLBuffer> pVertex;
	Unique<GLBuffer> pIndex;
};


struct CSFVertex
{
	Vector4 position;
	Vector4 normal;
};


struct CSFDrawRange
{
	CSFDrawRange()
		:geomIndex(0)
	{
	}
	int geomIndex;
	DrawArgs draw;
};


struct CSFSolid
{
	CSFSolid()
		: geomIndex(0)
		, matrixIndex(0)
		, materialIndex(0)
		, drawIndex(0)
	{
	}
	int geomIndex;
	int matrixIndex;
	int materialIndex;
	int drawIndex;
};

class CSFShader;
class CSFRenderNode : public RenderNode
{
public:
	CSFRenderNode(const String& name);
	virtual ~CSFRenderNode();
	friend class CSFRenderParts;

	struct CSFRenderParts : public RenderParts
	{
		enum Type
		{
			Face,
			Edge
		};
		CSFRenderParts(CSFRenderNode* pNode, Type _type, int _parts)
			: m_pNode(pNode)
			, type(_type)
			, parts(_parts)
		{

		}

		virtual String ToString();
		static const CSFRenderParts* Cast(const RenderParts* pParts);

		CSFRenderNode* m_pNode;
		Type type;
		int parts;
	};

	virtual void DrawPartsNode(const DrawContext& context, const RenderParts& parts);
	virtual bool CollectPickedNode(PickResult& result);
	virtual void PickNode(const PickContext& context);

	virtual void DrawNode(const DrawContext& context);
	virtual void ShowUI();

	void SetMaterial(Vector<CSFMaterial>&& materials) { m_materials = std::move(materials); UpdateMaterial(); }
	void SetNode(Vector<CSFNode>&& nodes) { m_nodes = std::move(nodes); UpdateNode(); }
	void SetMeshBuffer(Vector<Unique<CSFMeshBuffer>>&& mesh) { m_gpu.pMeshBuffer = std::move(mesh); }
	void SetSolids(Vector<CSFSolid>&& solid) { m_solids = std::move(solid);  m_needUpdateProperty = true; }
	void SetWireDraw(Vector<Vector<CSFDrawRange>>&& solid) { m_wireDraw = std::move(solid);  m_needUpdateProperty = true; }
	void SetSolidDraw(Vector<Vector<CSFDrawRange>>&& solid) { m_solidDraw = std::move(solid); m_needUpdateProperty = true; }

	void UpdateProperty();
private:
	void UpdateMaterial();
	void UpdateNode();
	struct UI
	{
		UI()
			: visible(true)
			, showWire(true)
			, visibleGeomIndex(-1)
		{
		}
		bool showWire;
		bool visible;
		int visibleGeomIndex;
	};

	struct GPU
	{
		Unique<GLBuffer> pMaterialBuffer;
		Unique<GLBuffer> pNodeBuffer;
		Vector<Unique<CSFMeshBuffer>> pMeshBuffer;
	};

	struct Property
	{
		Property()
			: triangleNum(0)
			, lineNum(0)
		{
		}
		int triangleNum;
		int lineNum;
	};

	bool m_needUpdateProperty;
	GPU m_gpu;
	UI m_ui;
	CSFShader* m_pShader;
	Vector<CSFSolid> m_solids;
	Vector<CSFMaterial> m_materials;
	Vector<CSFNode> m_nodes;
	Vector<Vector<CSFDrawRange>> m_wireDraw;
	Vector<Vector<CSFDrawRange>> m_solidDraw;
	Property m_property;
};

}
#endif KI_CSF_NODE_H
#ifndef KI_MESH_NODE_H
#define KI_MESH_NODE_H
#include "RenderNode.h"
#include "HalfEdgeStruct.h"
#include "MeshletGenerator.h"
namespace KI
{
class HalfEdgeNode : public RenderNode
{
public:
	HalfEdgeNode(const String& name, const Shared<HalfEdgeStruct>& pStruct);
	~HalfEdgeNode();

protected:
	virtual void ShowUI();
	virtual void DrawNode(const DrawContext& context);
	virtual void UpdateData(float time) {};
	
private:
	void ShowEdge();
	void BuildGLBuffer();
	Unique<GLBuffer> m_pPosition;
	Unique<GLBuffer> m_pFaceIndexBuffer;
	Unique<GLBuffer> m_pEdgeIndexBuffer;
	Shared<HalfEdgeStruct> m_pHalfEdge;
	
	struct MeshletGpu
	{
		Unique<MeshletShader> shader;
		Unique<GLBuffer> color;
		Unique<GLBuffer> position;
		Unique<GLBuffer> culster;
		Unique<GLBuffer> index;
	};

	MeshletGpu m_meshletGpu;

	struct UI
	{
		UI()
			: meshlet(0)
			, visibleMesh(true)
			, visibleEdge(false)
		{
		}
		int meshlet;
		bool visibleMesh;
		bool visibleEdge;
	};

	UI m_ui;
};


}


#endif KI_MESH_NODE_H
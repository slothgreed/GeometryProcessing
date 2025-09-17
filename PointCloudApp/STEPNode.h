#ifndef KI_STEP_NODE_H
#define KI_STEP_NODE_H
#include "RenderNode.h"
#include "Polyline.h"
namespace KI
{
struct STEPStruct;
class STEPLoader
{
public:
	STEPLoader() {};
	~STEPLoader() {};

	static RenderNode* Load(const String& name, bool saveOriginal = false);
private:
	static RenderNode* CreateRenderNode(const String& name, const STEPStruct& step);

};


struct STEPMesh
{
	Vector<Vector3> triangels;
	Vector<Vector3> edges;
	Vector<Polyline> polylines;
	Vector<Vector3> vertexs;
};


class STEPRenderNode : public RenderNode
{
public:
	STEPRenderNode(const String& name):RenderNode(name) {};
	virtual ~STEPRenderNode() {};

	virtual void DrawNode(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

	void SetMesh(Vector<STEPMesh>&& mesh) { m_mesh = std::move(mesh); }
private:
	void BuildGLResource();
	struct GPU
	{
		Unique<GLBuffer> pTriangles;
		Unique<GLBuffer> pEdges;
		Unique<GLBuffer> pVertexs;
	};

	GPU m_gpu;
	Vector<STEPMesh> m_mesh;
};

}
#endif KI_STEP_NODE_H
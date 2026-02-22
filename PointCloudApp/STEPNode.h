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

	static RenderNode* Load(const String& name, int index, bool saveOriginal = false);
private:
	static RenderNode* CreateRenderNode(const String& name, const Shared<STEPStruct>& step);

};


struct STEPMesh
{
	Vector<Vector3> triangels;
	Vector<Vector3> edges;
	Vector<Polyline> polylines;
	Vector<Vector3> vertexs;
	BDB CreateBDB()
	{
		BDB bdb;
		for (int i = 0; i < edges.size(); i++) {
			bdb.Add(edges[i]);
		}

		return bdb;
	}
};


class STEPRenderNode : public RenderNode
{
public:
	STEPRenderNode(const String& name, const Shared<STEPStruct>& step)
		: RenderNode(name)
		, m_rotateMatrix(Matrix4x4(1))
		, m_step(step){};
	virtual ~STEPRenderNode() {};

	virtual void DrawNode(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

	void SetMesh(Vector<STEPMesh>&& mesh) { m_mesh = std::move(mesh); }
private:
	virtual void UpdateData(float diff);
	void BuildGLResource();
	struct GPU
	{
		Unique<GLBuffer> pBDBLine;
		Unique<GLBuffer> pBDBLineIndex;

		Unique<GLBuffer> pTriangles;
		Unique<GLBuffer> pEdges;
		Unique<GLBuffer> pVertexs;
	};

	GPU m_gpu;
	Vector<STEPMesh> m_mesh;
	Matrix4x4 m_rotateMatrix;
	Shared<STEPStruct> m_step;
};

}
#endif KI_STEP_NODE_H
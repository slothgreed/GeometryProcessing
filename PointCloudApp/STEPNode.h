#ifndef KI_STEP_NODE_H
#define KI_STEP_NODE_H
#include "RenderNode.h"
#include "Mesh.h"
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


struct STEPShape
{
	Vector<Mesh> meshs;
	Vector<Polyline> polylines;
	//Vector<Vector3> triangels;
	//Vector<Vector3> edges;
	//Vector<Vector3> vertexs;
	BDB CreateBDB() const
	{
		BDB bdb;

		for (int i = 0; i < meshs.size(); i++) {
			bdb.Add(Mesh::CreateBDB(meshs[i]));
		}

		for (int i = 0; i < polylines.size(); i++) {
			bdb.Add(Polyline::CreateBDB(polylines[i]));
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

	void SetShape(Vector<STEPShape>&& shape) { m_shape = std::move(shape); }
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
	Vector<STEPShape> m_shape;
	Matrix4x4 m_rotateMatrix;
	Shared<STEPStruct> m_step;
};

}
#endif KI_STEP_NODE_H
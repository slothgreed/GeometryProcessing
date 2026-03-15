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

	struct UI
	{
		bool visibleBDB = false;
	};

	struct RenderBatch
	{
		int pointNum = 0;
		int indexNum = 0;
		Unique<GLBuffer> pPosition = nullptr;
		Unique<GLBuffer> pIndex = nullptr;
		GLuint drawType = GL_POINTS;
		void Allocate(GLuint type);
		bool IsActive() const { return pPosition != nullptr; }
	};

	struct GPU
	{
		RenderBatch bdb;

		bool IsActive() const
		{
			return
				triangle.IsActive() ||
				triangleIndex.IsActive() ||
				line.IsActive() ||
				lineIndex.IsActive() ||
				lineStrip.IsActive() ||
				lineStripIndex.IsActive() ||
				lineLoop.IsActive() ||
				lineLoopIndex.IsActive();
		}
		RenderBatch triangle;
		RenderBatch triangleIndex;
		RenderBatch line;
		RenderBatch lineIndex;
		RenderBatch lineStrip;
		RenderBatch lineStripIndex;
		RenderBatch lineLoop;
		RenderBatch lineLoopIndex;
	};

	GPU m_gpu;



	UI m_ui;
	Vector<STEPShape> m_shape;
	Matrix4x4 m_rotateMatrix;
	Shared<STEPStruct> m_step;
};

}
#endif KI_STEP_NODE_H
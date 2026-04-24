#ifndef KI_STEP_NODE_H
#define KI_STEP_NODE_H
#include "RenderNode.h"
#include "Mesh.h"
#include "Polyline.h"
#include "STEPEntity.h"
#include "STEPTypes.h"
namespace KI
{
class STEPLoader
{
public:
	STEPLoader() {};
	~STEPLoader() {};

	static STEPStruct* Load(const String& name, bool saveOriginal = false);
private:

};
struct EntityRange
{
	EntityRange() = default;
	EntityRange(int i, size_t f, size_t n) :id(i), first(f), num(n) {}
	bool IsEmpty()const { return id == 0; }
	bool IsActive()const { return !IsEmpty(); }
	int id = 0;
	size_t first = 0;
	size_t num = 0;
};


class STEPRenderNode : public RenderNode
{
public:
	STEPRenderNode(const String& name, const Shared<STEPStruct>& step)
		: RenderNode(name)
		, m_rotateMatrix(Matrix4x4(1))
		, m_step(step)
	{
		BuildShape();
	};
	virtual ~STEPRenderNode() {};

	void AddDebugNode(STEPUIContext& context, const STEPEntityBase* pBase);
	virtual void DrawNode(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

	void SetShape(Vector<STEPShape>&& shape) { m_shape = std::move(shape); }
private:
	void BuildShape();
	virtual void UpdateData(float diff);
	void BuildGLResource();

	struct UI
	{
		bool visibleBDB = false;
		bool visibleMesh = true;
	};


	struct RenderBatch
	{
		
		int pointNum = 0;
		int indexNum = 0;
		Unique<GLBuffer> pPosition = nullptr;
		Unique<GLBuffer> pIndex = nullptr;
		GLuint drawType = GL_POINTS;
		Vector<EntityRange> m_entity;
		void AddEntity(int id, size_t first, size_t num) { m_entity.push_back(EntityRange(id, first, num)); }
		EntityRange FindEntity(int id) const
		{
			if (id < 0) { return EntityRange(); }
			for (size_t i = 0; i < m_entity.size(); i++) {
				if (m_entity[i].id == id) {
					return m_entity[i];
				}
			}
			return EntityRange();
		}
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


	STEPUIContext uiContext;
	UI m_ui;
	Vector<STEPShape> m_shape;
	Matrix4x4 m_rotateMatrix;
	Shared<STEPStruct> m_step;
};

}
#endif KI_STEP_NODE_H
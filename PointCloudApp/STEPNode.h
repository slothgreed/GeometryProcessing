#ifndef KI_STEP_NODE_H
#define KI_STEP_NODE_H
#include "RenderNode.h"
#include "Mesh.h"
#include "Polyline.h"
namespace KI
{
struct STEPStruct;

struct STEPEntityBase;
class STEPRenderNode;
struct STEPUIContext
{
	bool IsSelect(int id) const;
	int GetSelectId() const;
	STEPRenderNode* pNode = nullptr;
	STEPEntityBase* pSelect = nullptr;
	UIContext* ui = nullptr;
};

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
	Vector<std::pair<int, Mesh>> meshs;
	Vector<std::pair<int, Polyline>> polylines;
	void AddMesh(int key, Mesh&& value)
	{
		meshs.push_back(std::pair<int, Mesh>(key, std::move(value)));
	}
	void AddPolyline(int key, Polyline&& value)
	{
		polylines.push_back(std::pair<int, Polyline>(key, std::move(value)));
	}
	BDB CreateBDB() const
	{
		BDB bdb;
		for (int i = 0; i < meshs.size(); i++) {
			bdb.Add(Mesh::CreateBDB(meshs[i].second));
		}

		for (int i = 0; i < polylines.size(); i++) {
			bdb.Add(Polyline::CreateBDB(polylines[i].second));
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

	void AddDebugNode(STEPUIContext& context, const STEPEntityBase* pBase);
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
		struct Entity
		{
			Entity() = default;
			Entity(int i, size_t f, size_t n) :id(i), first(f), num(n) {}
			bool IsEmpty()const { return id == 0; }
			bool IsActive()const { return !IsEmpty(); }
			int id = 0;
			size_t first = 0;
			size_t num = 0;
		};
		int pointNum = 0;
		int indexNum = 0;
		Unique<GLBuffer> pPosition = nullptr;
		Unique<GLBuffer> pIndex = nullptr;
		GLuint drawType = GL_POINTS;
		Vector<Entity> m_entity;
		void AddEntity(int id, size_t first, size_t num) { m_entity.push_back(Entity(id, first, num)); }
		Entity FindEntity(int id) const
		{
			if (id < 0) { return Entity(); }
			for (size_t i = 0; i < m_entity.size(); i++) {
				if (m_entity[i].id == id) {
					return m_entity[i];
				}
			}
			return Entity();
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
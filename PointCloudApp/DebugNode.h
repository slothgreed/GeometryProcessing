#ifndef DEBUG_NODE_H
#define DEBUG_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Texture.h"
#include "Primitive.h"
#include "Mesh.h"
#include "Polyline.h"
#include "DelaunayGenerator.h"
namespace KI
{

class DebugNode : public RenderNode
{
public:
	DebugNode(const String& name);
	~DebugNode() {};

	void DrawNode(const DrawContext& context);
	void SetPrimitive(const Shared<Primitive>& pPrimitive);
	void SetPrimitive(const std::vector<Shared<Primitive>>& pPrimitive);
protected:
	virtual void ShowUI(UIContext& ui) {};
private:
	struct UI
	{
		UI() : visible(true) {}
		bool visible;
	};

	UI m_ui;
	void BuildGLBuffer();

	struct Gpu
	{
		Unique<GLBuffer> pPosition;
		Unique<GLBuffer> pColor;
	};

	std::vector<Gpu> m_gpu;
	std::vector<Shared<Primitive>> m_pPrimitive;
};

class DelaunayDebugNode : public DebugNode
{
public:
	DelaunayDebugNode(const String& name);
	~DelaunayDebugNode() {};
	virtual void ShowUI(UIContext& ui);

private:

	struct UI
	{
		UI() :pattern(0) {}
		int pattern;
	};

	UI m_ui;
	void BuildDebugPrimitive();
	Vector<Shared<Primitive>> m_pPrimitive;
	DelaunayGenerator m_Delaunay;

};


class GridNode : public DebugNode
{
public:
	GridNode(const String& name, const Vector3& min, const Vector3& max, float interval);
	~GridNode() {};

private:

	Vector3 m_min;
	Vector3 m_max;
	float m_interval;
	Shared<Primitive> m_pPrimitive;

};

class PolylineNode : public RenderNode
{
public:
	PolylineNode(const String& name, const Polyline& polyline);
	~PolylineNode() = default;
	struct UI
	{
		bool visibleVertex = true;
		bool visibleTriangle = true;
		int selectedVertex = -1;
	};
	void BuildGLBuffer();
	virtual void DrawNode(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);
	void Set2D(bool value) { m_is2D = value; }

private:
	bool m_is2D = false;
	UI m_ui;
	Unique<GLBuffer> m_pPosition;
	Unique<GLBuffer> m_pIndex;
	Unique<GLBuffer> m_pTriPosition;
	Unique<GLBuffer> m_pUVPosition;
	Polyline m_polyline;
};

class MeshNode : public RenderNode
{
public:
	MeshNode(const String& name, const Mesh& mesh);
	~MeshNode() = default;
	struct UI
	{
		bool visibleVertex = true;
		bool visibleEdge = true;
	};
	void BuildGLBuffer();
	virtual void DrawNode(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

private:
	UI m_ui;
	Unique<GLBuffer> m_pPosition;
	Unique<GLBuffer> m_pIndex;
	Mesh m_mesh;
};

}

#endif DEBUG_NODE_H
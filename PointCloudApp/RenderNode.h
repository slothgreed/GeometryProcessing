#ifndef RENDER_NODE_H
#define RENDER_NODE_H
#include "RenderResource.h"
#include "BDB.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include "Camera.h"
namespace KI
{

struct DrawContext
{
	DrawContext(Camera* pCamera)
		:m_pCamera(pCamera)
	{
	}
	const Camera* m_pCamera;
};
class RenderNode
{
public:
	RenderNode(const String& name) : m_name(name) { m_matrix = Matrix4x4(1.0f); };
	RenderNode(const String& name, const Matrix4x4& matrix) : m_name(name), m_matrix(matrix) {}
	virtual ~RenderNode() {};

	RenderResource* GetResource();
	virtual void ShowUIData();
	virtual const BDB& GetBoundBox() { return m_bdb; }
	virtual void Draw(const Matrix4x4& proj, const Matrix4x4& view);
	virtual void Update(float time);
	void SetMatrix(const Matrix4x4& mat) { m_matrix = mat; }
	void SetBoundBox(const BDB& bdb) { m_bdb = bdb; }
	void RemoveNode(const String& name) { m_child.erase(name); }
	void AddNode(const Shared<RenderNode>& pNode) { m_child[pNode->m_name] = pNode; }
protected:
	virtual void ShowUI() {};
	virtual void DrawNode(const DrawContext& context) {};
	virtual void UpdateData(float time) {};
private:
	String m_name;
	Matrix4x4 m_matrix;
	BDB m_bdb;
	std::unordered_map<String, Shared<RenderNode>> m_child;
};
}

#endif RENDER_NODE_H
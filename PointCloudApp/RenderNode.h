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
class GLBuffer;
struct DrawContext
{
	DrawContext()
		: pCamera(nullptr)
		, gpuCamera(nullptr)
		, pShaderTable(nullptr)
	{
	}
	DrawContext(const Camera* _pCamera, const GLBuffer* _gpuCamera)
		: pCamera(_pCamera)
		, gpuCamera(_gpuCamera)
		, pShaderTable(nullptr){}
	const Camera* pCamera;
	const GLBuffer* gpuCamera;
	ShaderTable* pShaderTable;

};
class RenderNode
{
public:
	RenderNode(const String& name) : m_name(name) { m_matrix = Matrix4x4(1.0f); };
	RenderNode(const String& name, const Matrix4x4& matrix) : m_name(name), m_matrix(matrix) {}
	virtual ~RenderNode() {};

	virtual void ShowUIData();
	virtual const BDB& GetBoundBox() { return m_bdb; }
	virtual void Draw(const DrawContext& context);
	virtual void Update(float time);
	const Matrix4x4& GetMatrix() const { return m_matrix; }
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
#ifndef RENDER_NODE_H
#define RENDER_NODE_H
#include "RenderResource.h"
#include "BDB.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include "RenderResource.h"
#include "Light.h"
namespace KI
{
class GLBuffer;
struct DrawContext
{
	DrawContext()
		: pResource(nullptr)
	{
	}
	DrawContext(RenderResource* _pResource)
		: pResource(_pResource){}
	RenderResource* pResource;
};
class RenderNode
{
public:
	RenderNode(const String& name) : m_name(name) {
		m_scale = 1.0f;
		m_rotate = Vector3(0.0f);
		m_translate = Vector3(0.0f);
		m_matrix = Matrix4x4(1.0f); 
	};
	RenderNode(const String& name, const Matrix4x4& matrix) : m_name(name), m_matrix(matrix) {}
	virtual ~RenderNode() {};

	virtual void ShowUIData();
	virtual const BDB& GetBoundBox() { return m_bdb; }
	virtual void Draw(const DrawContext& context);
	virtual void Update(float time);
	const Matrix4x4& GetMatrix() const { return m_matrix; }
	void SetMatrix(float scale, const Vector3& rotate, const Vector3& translate);
	void SetMatrix(const Matrix4x4& mat) { m_matrix = mat; }
	void SetScale(float scale) { m_scale = scale; UpdateModelMatrix(); }
	void SetRotate(const Vector3& rotate) { m_rotate = rotate; UpdateModelMatrix(); }
	void SetRotateAngle(const Vector3& rotate);
	void SetTranslate(const Vector3& translate) { m_translate = translate; UpdateModelMatrix(); }
	void SetBoundBox(const BDB& bdb) { m_bdb = bdb; }
	void RemoveNode(const String& name) { m_child.erase(name); }
	void AddNode(const Shared<RenderNode>& pNode) { m_child[pNode->m_name] = pNode; }
	const Vector3& GetRotate() const { return m_rotate; }
	const Vector3& GetRotateAngle() const;
	float GetScale() { return m_scale; }
	const Vector3& GetTranslate() const { return m_translate; }
	const String& GetName() { return m_name; }
protected:
	virtual void ShowUI() {};
	virtual void DrawNode(const DrawContext& context) {};
	virtual void UpdateData(float time) {};
private:
	void UpdateModelMatrix();

	String m_name;
	float m_scale;
	Vector3 m_rotate;
	Vector3 m_translate;
	Matrix4x4 m_matrix;
	BDB m_bdb;
	std::unordered_map<String, Shared<RenderNode>> m_child;
};
}

#endif RENDER_NODE_H
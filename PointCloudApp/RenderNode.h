#ifndef RENDER_NODE_H
#define RENDER_NODE_H
#include "RenderResource.h"
class RenderNode
{
public:
	RenderNode(const String& name):m_name(name) {};
	~RenderNode() {};

	RenderResource* GetResource();
	virtual void ShowUIData();
	void Draw(const mat4x4& proj, const mat4x4& view);
	void RemoveNode(const String& name) { m_child.erase(name); }
	void SetNode(const Shared<RenderNode>& pNode) { m_child[pNode->m_name] = pNode; }
protected:
	virtual void ShowUI() {};
	virtual void DrawData(const mat4x4& proj, const mat4x4& view) {};
private:
	String m_name;
	std::unordered_map<String, Shared<RenderNode>> m_child;
};

#endif RENDER_NODE_H
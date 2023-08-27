#ifndef RENDER_NODE_H
#define RENDER_NODE_H

class RenderNode
{
public:
	RenderNode() {};
	~RenderNode() {};

	void Draw(const mat4x4& proj, const mat4x4& view);
	void AddNode(shared_ptr<RenderNode>& pNode) { m_child.push_back(pNode); }
protected:
	virtual void DrawData(const mat4x4& proj, const mat4x4& view) = 0;
private:
	std::vector<shared_ptr<RenderNode>> m_child;
};

#endif RENDER_NODE_H
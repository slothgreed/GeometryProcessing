#ifndef RENDER_NODE_H
#define RENDER_NODE_H

class RenderNode
{
public:
	RenderNode() {};
	~RenderNode() {};

	void AddNode(shared_ptr<RenderNode>& pNode) { m_child.push_back(pNode); }
private:
	std::vector<shared_ptr<RenderNode>> m_child;
};

#endif RENDER_NODE_H
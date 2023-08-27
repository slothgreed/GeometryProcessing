#include "RenderNode.h"

void RenderNode::Draw(const mat4x4& proj, const mat4x4& view)
{
	DrawData(proj, view);

	for (auto& data : m_child) {
		data->DrawData(proj, view);
	}
}

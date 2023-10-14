#include "RenderNode.h"
#include "PointCloudApp.h"
RenderResource* RenderNode::GetResource()
{
	return PointCloudApp::Application()->GetResource();
}
void RenderNode::ShowUIData()
{
	ShowUI();

	for (auto& data : m_child) {
		data.second->ShowUI();
	}
}
void RenderNode::Draw(const mat4x4& proj, const mat4x4& view)
{
	DrawData(proj, view);

	for (auto& data : m_child) {
		data.second->Draw(proj, view);
	}
}

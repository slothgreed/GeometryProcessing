#include "RenderNode.h"
#include "PointCloudApp.h"
namespace KI
{

void RenderNode::ShowUIData()
{
	ShowUI();

	for (auto& data : m_child) {
		data.second->ShowUI();
	}
}
void RenderNode::Draw(const DrawContext& context)
{
	DrawNode(context);

	for (auto& data : m_child) {
		data.second->Draw(context);
	}
}

void RenderNode::Update(float time)
{
	UpdateData(time);

	for (auto& data : m_child) {
		data.second->Update(time);
	}
}
}
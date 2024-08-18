#include "RenderNode.h"
#include "PointCloudApp.h"
#include "Utility.h"
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

const Vector3& RenderNode::GetRotateAngle() const
{
	return Vector3(glm::degrees(m_rotate.x), glm::degrees(m_rotate.y), glm::degrees(m_rotate.z));
}

void RenderNode::SetRotateAngle(const Vector3& rotate)
{
	m_rotate = Vector3(glm::radians(rotate.x), glm::radians(rotate.y), glm::radians(rotate.z));
	
	
	UpdateModelMatrix();
}
void RenderNode::SetMatrix(float scale, const Vector3& rotate, const Vector3& translate)
{
	m_scale = scale;
	m_translate = translate;
	m_rotate = rotate;
	UpdateModelMatrix();
}
void RenderNode::UpdateModelMatrix()
{
	SetMatrix(
		glmUtil::CreateTranslate(m_translate) *
		glmUtil::CreateRotate(m_rotate) *
		glmUtil::CreateScale(m_scale));
}
}
#include "RenderNode.h"
#include "PointCloudApp.h"
#include "Utility.h"
namespace KI
{

void RenderNode::ShowUIData(UIContext& ui)
{
	ShowUI(ui);

	for (auto& data : m_child) {
		data.second->ShowUI(ui);
	}

}

void RenderNode::Draw(const DrawContext& context)
{
	DrawNode(context);

	for (auto& data : m_child) {
		data.second->Draw(context);
	}
}

void RenderNode::DrawParts(const DrawContext& context, const RenderParts& parts)
{
	DrawPartsNode(context, parts);
}

void RenderNode::Pick(const PickContext& context)
{
	PickNode(context);
	for (auto& data : m_child) {
		data.second->Pick(context);
	}
}


void RenderNode::CollectPicked(PickResult& result)
{
	CollectPickedNode(result);
	for (auto& data : m_child) {
		data.second->CollectPickedNode(result);
	}
}


void RenderNode::Update(float time)
{
	UpdateData(time);

	for (auto& data : m_child) {
		data.second->Update(time);
	}
}

Vector3 RenderNode::GetRotateAngle() const
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

void RenderNode::SetMatrix(const Matrix4x4& mat)
{
	m_matrix = mat;
	m_normalMatrix = glm::transpose(glm::inverse(glm::mat3(mat)));
}


BDB RenderNode::GetCameraFitBox() const
{
	return m_bdb.CreateRotate(m_matrix);
}
BDB RenderNode::CalcCameraFitBox()
{
	BDB bdb;
	bdb.Add(GetCameraFitBox());
	for (const auto& child : m_child) {
		auto pRenderNode = child.second.get();
		bdb.Add(pRenderNode->GetCameraFitBox());
		pRenderNode->CalcCameraFitBox(bdb);
	}

	return bdb;
}

BDB RenderNode::CalcCameraFitBox(BDB bdb)
{
	for (const auto& child : m_child) {
		auto pRenderNode = child.second.get();
		bdb.Add(pRenderNode->GetCameraFitBox());
		pRenderNode->CalcCameraFitBox(bdb);
	}

	return bdb;
}

void RenderNode::ShowUIParameter(const Parameter& parameter, UIContext& ui)
{
	static int binCount = 50; // ビン数（初期値）
	static bool logScale = false;


	// 統計量の計算
	float minArea = parameter.Min();
	float maxArea = parameter.Max();
	float sum = parameter.Sum();
	float mean = parameter.Average();

	auto histogram = parameter.CreateHistogram(logScale, binCount);
	// UI
	ImGui::Begin(parameter.Name().data());

	ImGui::Text("Count: %d", (int)parameter.Size());
	ImGui::Text("Min: %.5f", minArea);
	ImGui::Text("Max: %.5f", maxArea);
	ImGui::Text("Mean: %.5f", mean);
	//ImGui::Text("Median: %.5f", median);
	ImGui::Checkbox("Log scale", &logScale);
	ImGui::SliderInt("Bin count", &binCount, 10, 200);

	// 最大値（縦軸スケール）取得
	float maxY = *std::max_element(histogram.begin(), histogram.end());

	ImGui::PlotHistogram("Area Histogram", histogram.data(), binCount, 0,
		nullptr, 0.0f, maxY, ImVec2(400, 150));

	ImGui::End();
}

}
#include "SignedDistanceField.h"
#include "HalfEdgeNode.h"
#include "Primitives.h"
#include "PrimitiveNode.h"
#include "Texture.h"
#include "Utility.h"
namespace KI
{
SignedDistanceField::SignedDistanceField(HalfEdgeNode* pHalfEdge)
	:m_pHalfEdge(pHalfEdge)
	,m_resolute(16)
{
	auto sampler = Texture::Sampler(Texture::Sampler::FILTER::NEAREST);
	m_gpu.xTexture = std::make_shared<Texture2D>(sampler);
	m_gpu.yTexture = std::make_shared<Texture2D>(sampler);
	m_gpu.zTexture = std::make_shared<Texture2D>(sampler);
}

SignedDistanceField::~SignedDistanceField()
{
}

void SignedDistanceField::Execute()
{

}

SignedDistanceField::UI::UI()
{
	glStatus = std::make_shared<GLStatus>();
	glStatus->SetBackCull(false);
	resolute = 0;

}

BVH::IntersectResult SignedDistanceField::CalcMinDistance(const Vector3& pos) const
{
	return m_pHalfEdge->GetBVH()->CalcMinDistance(pos);
}

void SignedDistanceField::CreateSDFTexture(int resolute, float position, Axis axis, std::vector<Vector3>& lines)
{
	auto bdb = m_pHalfEdge->GetBoundBox();
	auto diag = bdb.Max() - bdb.Min();
	auto pitch = diag / (float)resolute;
	auto halfPitch = 0.5f * pitch;
	std::vector<unsigned char> image(resolute * resolute * 4);
	Vector3 pixelPos;
	auto maxLength = bdb.MaxLength();
	if (axis == Axis::X) { pixelPos.x = position; }
	if (axis == Axis::Y) { pixelPos.y = position; }
	if (axis == Axis::Z) { pixelPos.z = position; }

	for (int i = 0; i < resolute; i++) {
		if (axis == Axis::X) { pixelPos.y = bdb.Min().y + (i * pitch.y) + halfPitch.y; }
		if (axis == Axis::Y) { pixelPos.z = bdb.Min().z + (i * pitch.z) + halfPitch.z; }
		if (axis == Axis::Z) { pixelPos.x = bdb.Min().x + (i * pitch.x) + halfPitch.x; }

		for (int j = 0; j < resolute; j++) {
			if (axis == Axis::X) { pixelPos.z =  bdb.Min().z + (j * pitch.z) + halfPitch.z; }
			if (axis == Axis::Y) { pixelPos.x =  bdb.Min().x + (j * pitch.x) + halfPitch.x; }
			if (axis == Axis::Z) { pixelPos.y =  bdb.Min().y + (j * pitch.y) + halfPitch.y; }
			auto pixel = j + (i * resolute);

			auto minDist = CalcMinDistance(pixelPos);
			auto dist = minDist.distance * 255 / maxLength;
			float frequency = 5.0f;
			dist = abs(mod(minDist.distance, frequency) - (frequency * 0.5f));
			if (dist < 0.1f) {
				image[4 * pixel] = 255;
				image[4 * pixel + 1] = 255;
				image[4 * pixel + 2] = 255;
				image[4 * pixel + 3] = 255;
			} else {
				image[4 * pixel] = 0;
				image[4 * pixel + 1] = 0;
				image[4 * pixel + 2] = 0;
				image[4 * pixel + 3] = 255;
			}

			//lines.push_back(pixelPos);
			//lines.push_back(minDist.position);
		}
	}

	if (axis == Axis::X) { m_gpu.xTexture->Build(resolute, resolute, image.data()); }
	if (axis == Axis::Y) { m_gpu.yTexture->Build(resolute, resolute, image.data()); }
	if (axis == Axis::Z) { m_gpu.zTexture->Build(resolute, resolute, image.data()); }
}


void SignedDistanceField::CreateTexure(int resolute)
{
	if (m_gpu.xTexture->Size().x == resolute) { return; }
	m_gpu.xTexture->Resize(resolute, resolute);
	m_gpu.yTexture->Resize(resolute, resolute);
	m_gpu.zTexture->Resize(resolute, resolute);
}
void SignedDistanceField::ShowUI(UIContext& ui)
{
	
	const char* resolutions[] = { "4", "16", "32", "64", "128", "256", "512", "1024" };
	if (ImGui::Combo("Resolution", &m_ui.resolute, resolutions, IM_ARRAYSIZE(resolutions))) {
		if (m_ui.resolute == 0) { m_resolute = 4; } 
		else if (m_ui.resolute == 1) { m_resolute = 16; } 
		else if (m_ui.resolute == 2) { m_resolute = 32; } 
		else if (m_ui.resolute == 3) { m_resolute = 64; }
		else if (m_ui.resolute == 4) { m_resolute = 128; }
		else if (m_ui.resolute == 5) { m_resolute = 256; }
		else if (m_ui.resolute == 6) { m_resolute = 512; }
		else if (m_ui.resolute == 7) { m_resolute = 1024; }
		CreateTexure(m_resolute);
	}

	ImGui::Checkbox("VisibleX", &m_ui.xPlane.visible);
	const auto& bdb = m_pHalfEdge->GetBoundBox();
	if (m_ui.xPlane.visible) {
		if (ImGui::SliderFloat("XPlane", &m_ui.xPlane.position, bdb.Min().x, bdb.Max().x, "%lf", 1.0f)) {
			std::vector<Vector3> lines;
			std::shared_ptr<Primitive> pPlane = std::make_shared<Plane>(bdb.Min(), bdb.Max(), m_ui.xPlane.position, Plane::X, true);
			CreateSDFTexture(m_resolute, m_ui.xPlane.position, Axis::X, lines);
			auto pNode = std::make_shared<PrimitiveNode>("XPlane", pPlane, m_gpu.xTexture);
			pNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pNode->SetPickTarget(true);
			pNode->SetGLStatus(m_ui.glStatus);
			m_pHalfEdge->AddNode(pNode);


			auto pLine = std::make_shared<Primitive>();
			pLine->SetPosition(std::move(lines));
			pLine->SetType(GL_LINES);
			auto pLineNode = std::make_shared<PrimitiveNode>("XIntersect", pLine, ColorUtility::CreatePrimary(0));
			pLineNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pLineNode->SetGLStatus(m_ui.glStatus);
			m_pHalfEdge->AddNode(pLineNode);

		}
	} else {
		m_pHalfEdge->RemoveNode("XPlane");
		m_pHalfEdge->RemoveNode("XIntersect");
	}

	ImGui::Checkbox("VisibleY", &m_ui.yPlane.visible);
	if (m_ui.yPlane.visible) {
		if (ImGui::SliderFloat("YPlane", &m_ui.yPlane.position, bdb.Min().y, bdb.Max().y, "%lf", 1.0f)) {
			std::shared_ptr<Primitive> pPlane = std::make_shared<Plane>(bdb.Min(), bdb.Max(), m_ui.yPlane.position, Plane::Y, true);
			std::vector<Vector3> lines;
			CreateSDFTexture(m_resolute, m_ui.yPlane.position, Axis::Y, lines);
			auto pNode = std::make_shared<PrimitiveNode>("YPlane", pPlane, m_gpu.yTexture);
			pNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pNode->SetGLStatus(m_ui.glStatus);
			pNode->SetPickTarget(true);
			m_pHalfEdge->AddNode(pNode);


			auto pLine = std::make_shared<Primitive>();
			pLine->SetPosition(std::move(lines));
			pLine->SetType(GL_LINES);
			auto pLineNode = std::make_shared<PrimitiveNode>("YIntersect", pLine, ColorUtility::CreatePrimary(0));
			pLineNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pLineNode->SetGLStatus(m_ui.glStatus);
			m_pHalfEdge->AddNode(pLineNode);
		}
	} else {
		m_pHalfEdge->RemoveNode("YPlane");
		m_pHalfEdge->RemoveNode("YIntersect");
	}

	ImGui::Checkbox("VisibleZ", &m_ui.zPlane.visible);
	if (m_ui.zPlane.visible) {
		if (ImGui::SliderFloat("ZPlane", &m_ui.zPlane.position, bdb.Min().z, bdb.Max().z, "%lf", 1.0f)) {
			std::shared_ptr<Primitive> pPlane = std::make_shared<Plane>(bdb.Min(), bdb.Max(), m_ui.zPlane.position, Plane::Z, true);
			std::vector<Vector3> lines;
			CreateSDFTexture(m_resolute, m_ui.zPlane.position, Axis::Z,lines);
			auto pNode = std::make_shared<PrimitiveNode>("ZPlane", pPlane, m_gpu.zTexture);
			pNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pNode->SetGLStatus(m_ui.glStatus);
			pNode->SetPickTarget(true);
			m_pHalfEdge->AddNode(pNode);

			auto pLine = std::make_shared<Primitive>();
			pLine->SetPosition(std::move(lines));
			pLine->SetType(GL_LINES);
			auto pLineNode = std::make_shared<PrimitiveNode>("ZIntersect", pLine, ColorUtility::CreatePrimary(0));
			pLineNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pLineNode->SetGLStatus(m_ui.glStatus);
			m_pHalfEdge->AddNode(pLineNode);
		}
	} else {
		m_pHalfEdge->RemoveNode("ZPlane");
		m_pHalfEdge->RemoveNode("ZIntersect");
	}


}
}

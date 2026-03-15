#include "ImageNode.h"
#include "ImageAlgorithm.h"
#include "Primitives.h"
#include "DebugNode.h"
#include "PrimitiveNode.h"
#include "PostEffect.h"
namespace KI
{
ImageNode::ImageNode(const String& name, const Shared<Texture>& pTexture)
	:RenderNode(name)
{
	m_pTexture = pTexture;
}

void ImageNode::DrawNode(const DrawContext& context)
{
	if (!m_ui.visible) { return; }
	TextureDrawer::Execute(context, m_pTexture.get());
}

void ImageNode::ShowUI(UIContext& ui)
{
	ImGui::Checkbox("ImageNode::Visible", &m_ui.visible);
	
	bool notifyCreateOutline = false;
	if (ImGui::Checkbox("CreateOutline",&m_ui.outline)) {
		if (!m_ui.outline) {
			notifyCreateOutline = true;
		} else {
			RemoveNode("Outline");
		}
	}
	if (m_ui.outline || notifyCreateOutline) {
		if (ImGui::SliderInt("OutlineDebugCount", &m_ui.outlineDebugCount, -1, 100)) {
			auto polyline = ImageAlgorithm::CreateOutlineByMooreNeighbor(m_pTexture->GetPixelData(), m_ui.outlineDebugCount * 10)
				.CreateSmooth();
			auto ratio = Vector3(
				(float)ui.GetViewport().Size.x / m_pTexture->Size().x,
				(float)ui.GetViewport().Size.y / m_pTexture->Size().y, 1.0);
			for (int i = 0; i < polyline.GetPoints().size(); i++) {
				polyline.Set(i, polyline.GetPoints()[i] * ratio);
			}
			auto pPrimitive = std::make_shared<PolylineNode>("Outline", polyline);
			pPrimitive->Set2D(true);
			AddNode(pPrimitive);
			/*
			{
				auto pPrimitive = std::make_shared<PrimitiveNode>("Outline", ToPrimitive(polyline), Vector3(1, 0, 0));
				pPrimitive->Set2D(true);
				AddNode(pPrimitive);
			}

			{
				auto p = std::make_shared<Primitive>();
				p->SetPosition(polyline.CreateTriangleArray());
				p->SetType(GL_TRIANGLES);
				auto pPrimitive = std::make_shared<PrimitiveNode>("OutlineTriangle", p, Vector3(0, 0, 1));
				pPrimitive->Set2D(true);
				AddNode(pPrimitive);
			}

			{
				auto p = std::make_shared<Primitive>();
				p->SetPosition(polyline.CreateTriangleLine());
				p->SetType(GL_LINES);
				auto pPrimitive = std::make_shared<PrimitiveNode>("OutlineTriangleLine", p, Vector3(1, 0, 0));
				pPrimitive->Set2D(true);
				AddNode(pPrimitive);
			}


			{
				auto p = ToPrimitive(polyline);
				p->SetType(GL_POINTS);
				auto pPrimitive = std::make_shared<PrimitiveNode>("OutlinePoint", p, Vector3(0, 1, 0));
				pPrimitive->Set2D(true);
				AddNode(pPrimitive);
			}
			*/
		}
	}
}
}
#include "RenderPass.h"
namespace KI
{
void IRenderPass::Execute(RenderNode* pRoot, DrawContext& context) const
{
	if (!pRoot || !context.pResource) { return; }

	context.SetRenderPass(GetType());
	Begin(context);
	pRoot->Draw(context);
	End(context);
}

void DepthPrePass::Begin(DrawContext& context) const
{
	auto pGL = context.pResource->GL();
	pGL->EnableDepth();
	pGL->SetDepthFunc(GL_LESS);
	pGL->DepthMask(true);
	pGL->ColorMask(false);
	pGL->DisableBlend();
	pGL->EnablePolygonFill();
}

void DepthPrePass::End(DrawContext& context) const
{
	context.pResource->GL()->ColorMask(true);
}

void DefaultPass::Begin(DrawContext& context) const
{
	auto pGL = context.pResource->GL();
	pGL->EnableDepth();
	pGL->SetDepthFunc(GL_LEQUAL);
	pGL->DepthMask(true);
	pGL->ColorMask(true);
	pGL->DisableBlend();
	pGL->EnablePolygonFill();
}
}

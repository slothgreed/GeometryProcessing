#ifndef RENDER_PASS_H
#define RENDER_PASS_H
#include "RenderNode.h"
namespace KI
{
class IRenderPass
{
public:
	virtual ~IRenderPass() {}
	void Execute(RenderNode* pRoot, DrawContext& context) const;

protected:
	virtual RenderPassType GetType() const = 0;
	virtual void Begin(DrawContext& context) const = 0;
	virtual void End(DrawContext& context) const {}
};

class DepthPrePass : public IRenderPass
{
protected:
	virtual RenderPassType GetType() const override { return RenderPassType::DEPTH_PRE_PASS; }
	virtual void Begin(DrawContext& context) const override;
	virtual void End(DrawContext& context) const override;
};

class DefaultPass : public IRenderPass
{
protected:
	virtual RenderPassType GetType() const override { return RenderPassType::DEFAULT_PASS; }
	virtual void Begin(DrawContext& context) const override;
};
}
#endif RENDER_PASS_H

#include "GL/GLContext.h"
#include "GL/Buffer/FrameBuffer.h"
#include "GL/RenderTarget.h"
namespace KI
{

Viewport Viewport::Create(const Vector2i& windowSize, const Vector2i& ratioSize, Viewport::Anchor anchor)
{
	int w = windowSize.x / ratioSize.x;
	int h = windowSize.y / ratioSize.y;

	int x = 0;
	int y = 0;
	switch (anchor) {
	case Viewport::Anchor::TopLeft:
		x = 0;
		y = windowSize.y - h;
		break;
	case Anchor::TopRight:
		x = windowSize.x - w;
		y = windowSize.y - h;
		break;
	case Anchor::BottomLeft:
		x = 0;
		y = 0;
		break;
	case Anchor::BottomRight:
		x = windowSize.x - w;
		y = 0;
		break;
	}
	return Vector4i(x, y, w, h);
}

std::array<Viewport, 2> Viewport::SplitHorizontal(const Viewport& viewport)
{
	const auto& v = viewport.Get();
	const int leftW = v.z / 2;
	const int rightW = v.z - leftW;

	return
	{
		Viewport(Vector4i(v.x,         v.y, leftW,  v.w)),
		Viewport(Vector4i(v.x + leftW, v.y, rightW, v.w))
	};
}
std::array<Viewport, 2> Viewport::SplitVertical(const Viewport& viewport)
{
	const auto& v = viewport.Get();
	const int bottomH = v.w / 2;
	const int topH = v.w - bottomH;

	return
	{
		Viewport(Vector4i(v.x, v.y + bottomH, v.z, topH)),
		Viewport(Vector4i(v.x, v.y,           v.z, bottomH))
	};
}


void GLContext::SetupStatus(const GLStatus& status)
{
	if (status.backCull) {
		EnableCullFace();
	} else {
		DisableCullFace();
	}

	SetPointSize(status.pointSize);
	SetLineWidth(status.lineWidth);
}

void GLContext::SetWindowSize(const Vector2i& size)
{
	m_windowSize = size;
	SetViewport(m_windowSize);
}

void GLContext::Clear(GLuint clear)
{
	glClear(clear);
}

Viewport GLContext::CreateViewport(const Vector2i& ratioSize, Viewport::Anchor anchor) const
{
	return Viewport::Create(m_windowSize, ratioSize, anchor);
}
void GLContext::SetViewport(const Viewport& viewport)
{
	const auto& value = viewport.Get();
	glViewport(value.x, value.y, value.z, value.w);
}

void GLContext::SetViewportFullWindow()
{
	SetViewport(m_windowSize);
}

void GLContext::EnableScissor(const Viewport& value)
{
	glEnable(GL_SCISSOR_TEST);
	const auto& scissor = value.Get();
	glScissor(scissor.x, scissor.y, scissor.z, scissor.w);
}
void GLContext::DisableScissor()
{
	glDisable(GL_SCISSOR_TEST);
}



void GLContext::EnablePolygonOffset(int factor, int units)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(factor, units);
}

void GLContext::EnableCullFace()
{
	glEnable(GL_CULL_FACE);
}

void GLContext::EnablePolygonWire()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}
void GLContext::EnablePolygonFill()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void GLContext::EnableBlend()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void GLContext::DisableBlend()
{
	glDisable(GL_BLEND);
}
void GLContext::DisableCullFace()
{
	glDisable(GL_CULL_FACE);
}
void GLContext::SetCullFace(GLenum face)
{
	glCullFace(face);
}
void GLContext::EnableDepth()
{
	glEnable(GL_DEPTH_TEST);
}
void GLContext::DisableDepth()
{
	glDisable(GL_DEPTH_TEST);
}
void GLContext::SetDepthFunc(GLenum func)
{
	glDepthFunc(func);
}
void GLContext::DepthMask(bool value)
{
	glDepthMask(value ? GL_TRUE : GL_FALSE);
}
void GLContext::EnableStencil()
{
	glEnable(GL_STENCIL_TEST);
}
void GLContext::DisableStencil()
{
	glDisable(GL_STENCIL_TEST);
}
void GLContext::EnableClipDistance(int index)
{
	glEnable(GL_CLIP_DISTANCE0 + index);
}
void GLContext::DisableClipDistance(int index)
{
	glDisable(GL_CLIP_DISTANCE0 + index);
}
void GLContext::SetStencilFunc(GLenum func, int reference, GLuint mask)
{
	glStencilFunc(func, reference, mask);
}
void GLContext::SetStencilOperation(GLenum stencilFail, GLenum depthFail, GLenum depthPass)
{
	glStencilOp(stencilFail, depthFail, depthPass);
}
void GLContext::SetStencilOperationSeparate(GLenum face, GLenum stencilFail, GLenum depthFail, GLenum depthPass)
{
	glStencilOpSeparate(face, stencilFail, depthFail, depthPass);
}
void GLContext::SetStencilMask(GLuint mask)
{
	glStencilMask(mask);
}
void GLContext::SetClearStencil(int value)
{
	glClearStencil(value);
}
void GLContext::DisablePolygonOffset()
{
	glDisable(GL_POLYGON_OFFSET_FILL);
}
void GLContext::SetPointSize(float value)
{
	if (m_cache.pointSize == value) { return; }
	if (value < 0) { return; }
	glPointSize(value);
	OUTPUT_GLERROR;
	m_cache.pointSize = value;
}

void GLContext::SetLineWidth(float value)
{
	if (m_cache.lineWidth == value) { return; }
	if (value < 0) { return; }
	glLineWidth(value);
	OUTPUT_GLERROR;
	m_cache.lineWidth = value;
}

void GLContext::SetupPick()
{
	SetPointSize(8.0f);
	SetLineWidth(7.0f);
}
void GLContext::SetupShading()
{
	SetPointSize(8.0f);
	SetLineWidth(3.0f);
}


void GLContext::PushRenderTarget(RenderTarget* pTarget, int drawTargetNum)
{
	RenderTargetStack stack;
	stack.pRenderTarget = pTarget;
	stack.drawTargetNum = drawTargetNum;
	m_pRenderTargetStack.push(stack);
	pTarget->Bind(drawTargetNum);
}

void GLContext::ColorMask(bool value)
{
	if (value) {
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	} else {
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	}
}
void GLContext::PopRenderTarget()
{
	if (m_pRenderTargetStack.empty()) {
		FrameBuffer::UnBind();
		return;
	}
	m_pRenderTargetStack.pop();
	if (m_pRenderTargetStack.empty()) {
		FrameBuffer::UnBind();
		return;
	}

	auto pTarget = m_pRenderTargetStack.top();

	pTarget.pRenderTarget->Bind(pTarget.drawTargetNum);
}
}

#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

namespace KI
{
class RenderTarget;
struct GLStatus
{
	friend class GLContext;
	GLStatus()
		:backCull(true)
		, pointSize(-1.0f)
		, lineWidth(-1.0f)
	{
	}

	void SetPointSize(float size) { lineWidth = size; }
	void SetLineWidth(float size) { pointSize = size; }
	void SetBackCull(bool value) { backCull = value; }
private:
	float pointSize;
	float lineWidth;
	bool backCull;
};

struct Viewport
{
	Viewport(const Vector2i& size) :m_value(Vector4i(0, 0, size.x, size.y)) {}
	Viewport(const Vector4i& value) :m_value(value) {}
	enum Anchor
	{
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

	const Vector4i& Get() const { return m_value; }
	const int Width() const { return m_value.z - m_value.x; }
	const int Height() const { return m_value.w - m_value.y; }
	static Viewport Create(const Vector2i& windowSize, const Vector2i& ratioSize, Anchor anghor);
	static std::array<Viewport, 2> SplitHorizontal(const Viewport& viewport);
	static std::array<Viewport, 2> SplitVertical(const Viewport& viewport);
private:
	Vector4i m_value;
};


class GLContext
{
public:
	GLContext() {}
	~GLContext() {};

	void SetupStatus(const GLStatus& status);
	void EnablePolygonOffset(int factor, int units);
	void DisablePolygonOffset();
	void EnableDepth();
	void DisableDepth();
	void SetDepthFunc(GLenum func);
	void DepthMask(bool value);
	void EnableCullFace();
	void DisableCullFace();
	void SetCullFace(GLenum face);
	void EnableBlend();
	void DisableBlend();
	void EnableStencil();
	void DisableStencil();
	void EnableClipDistance(int index);
	void DisableClipDistance(int index);
	void SetStencilFunc(GLenum func, int reference, GLuint mask);
	void SetStencilOperation(GLenum stencilFail, GLenum depthFail, GLenum depthPass);
	void SetStencilOperationSeparate(GLenum face, GLenum stencilFail, GLenum depthFail, GLenum depthPass);
	void SetStencilMask(GLuint mask);
	void SetClearStencil(int value);
	void EnablePolygonWire();
	void EnablePolygonFill();
	void EnableScissor(const Viewport& scissor);
	void DisableScissor();


	void SetWindowSize(const Vector2i& size);

	void Clear(GLuint clear);

	Viewport CreateViewport(const Vector2i& ratioSize, Viewport::Anchor anghor) const;
	void SetViewport(const Viewport& viewport);
	void SetViewportFullWindow();
	void SetPointSize(float value);
	void SetLineWidth(float value);

	void SetupPick();
	void SetupShading();

	void PushRenderTarget(RenderTarget* pTarget, int drawTargetNum = -1);
	void PopRenderTarget();
	void ColorMask(bool value);
	const Vector2i& GetWindowSize() const { return m_windowSize; }
private:
	Vector2i m_windowSize = Vector2i(0, 0);
	GLStatus m_cache;

	struct RenderTargetStack
	{
		RenderTarget* pRenderTarget = nullptr;
		int drawTargetNum = 0;
	};


	std::stack<RenderTargetStack> m_pRenderTargetStack;
};
}

#endif GL_CONTEXT_H

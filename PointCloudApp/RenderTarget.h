#ifndef RENDER_TEXTURE
#define	RENDER_TEXTURE
#include "FrameBuffer.h"
#include "Texture.h"
namespace KI
{
class RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();

	void Resize(const Vector2& size);
	void Build(Shared<Texture2D>&& pColor);
	void Build(Shared<Texture2D>&& pColor, Shared<Texture2D>&& pDepth);
	void Build(std::vector<Shared<Texture2D>>&& pColor, Shared<Texture2D>&& pDepth);

	void Bind(int drawTarget = -1);
	void UnBind();

	int GetIntPixel(int x, int y);
	float GetDepth(int x, int y);
	int GetColorNum() const { return m_pColor.size(); }
	int GetDepthNum() const { return m_pDepth == nullptr ? 0 : 1; }
	const Shared<Texture2D>& GetColor(int index) const { return m_pColor[index]; }
	const Shared<Texture2D>& GetPosition() const { return m_pColor[1]; }
	const Shared<Texture2D>& GetNormal() const { return m_pColor[2]; }
	const Shared<Texture2D>& GetDepth() const { return m_pDepth; }
	static RenderTarget* CreatePickTarget(const Vector2i& size);
	static RenderTarget* CreateForwardTarget(const Vector2i& size);
	static RenderTarget* CreatePostEffectTarget(const Vector2i& size);
	static Texture2D* CreatePickTexture(const Vector2i& size);
	static Texture2D* CreateColorTexture(const Vector2i& size);
	static Texture2D* CreateColorFloatTexture(const Vector2i& size);
	static Texture2D* CreateDepthTexture(const Vector2i& size);
	void Copy(const RenderTarget& target);
private:
	void Build();
	Unique<FrameBuffer> m_pFrameBuffer;
	std::vector<Shared<Texture2D>> m_pColor;
	std::vector<GLenum> m_attachment;
	Shared<Texture2D> m_pDepth;
};
}


#endif RENDER_TEXTURE
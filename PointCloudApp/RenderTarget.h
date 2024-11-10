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
	void Build(const Shared<Texture2D>& pColor, const Shared<Texture2D>& pDepth);

	void Bind();
	void UnBind();

	int GetIntPixel(int x, int y);
	const Shared<Texture2D>& GetColor(int index) const { return m_pColor[index]; }
	const Shared<Texture2D>& GetDepth() const { return m_pDepth; }
	static RenderTarget* CreatePickTarget(const Vector2& size);
	static Texture2D* CreatePickTexture(int width, int height);
	static Texture2D* CreateColorTexture(int width, int height);
	static Texture2D* CreateDepthTexture(int width, int height);
private:
	Unique<FrameBuffer> m_pFrameBuffer;
	std::vector<Shared<Texture2D>> m_pColor;
	std::vector<GLenum> m_attachment;
	Shared<Texture2D> m_pDepth;
};
}


#endif RENDER_TEXTURE
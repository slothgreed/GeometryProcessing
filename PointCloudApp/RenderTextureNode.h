#ifndef RENDER_TEXTURE_NODE_H
#define RENDER_TEXTURE_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Primitives.h"
#include "Texture.h"
namespace KI
{

class RenderTextureNode : public RenderNode
{
public:
	RenderTextureNode(const String& name, const Shared<Texture>& pTexture);
	~RenderTextureNode() {};

	void DrawNode(const DrawContext& context);

private:
	void BuildGLBuffer();
	void UpdateRenderData();
	Shared<TextureShader> m_pShader;
	Shared<Texture> m_pTexture;
	RenderPlane m_pPrimitive;
	Unique<GLBuffer> m_pPositionBuffer;
	Unique<GLBuffer> m_pTexcoordBuffer;
	Unique<GLBuffer> m_pIndexBuffer;
};

}

#endif RENDER_TEXTURE_NODE_H
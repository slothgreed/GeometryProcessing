#ifndef RENDER_TEXTURE_NODE_H
#define RENDER_TEXTURE_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Primitives.h"
#include "Texture.h"
class RenderTextureNode : public RenderNode
{
public:
	RenderTextureNode(const String& name, const Shared<Texture>& pTexture);
	~RenderTextureNode() {};

	void DrawData(const mat4x4& proj, const mat4x4& view);

private:
	void BuildGLBuffer();
	void UpdateRenderData();
	Shared<TextureShader> m_pShader;
	Shared<Texture> m_pTexture;
	RenderPlane m_pPrimitive;
	std::unique_ptr<GLBuffer> m_pPositionBuffer;
	std::unique_ptr<GLBuffer> m_pTexcoordBuffer;
	std::unique_ptr<GLBuffer> m_pIndexBuffer;
};

#endif RENDER_TEXTURE_NODE_H
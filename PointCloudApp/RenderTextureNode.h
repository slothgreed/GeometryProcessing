#ifndef RENDER_TEXTURE_NODE_H
#define RENDER_TEXTURE_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Primitives.h"
#include "Texture.h"
class RenderTextureNode : public RenderNode
{
public:
	RenderTextureNode(const string& name, const std::shared_ptr<Texture>& pTexture);
	~RenderTextureNode() {};

	void DrawData(const mat4x4& proj, const mat4x4& view);

private:
	void BuildGLBuffer();
	void UpdateRenderData();
	std::shared_ptr<TextureShader> m_pShader;
	std::shared_ptr<Texture> m_pTexture;
	RenderPlane m_pPrimitive;
	std::unique_ptr<GLBuffer> m_pPositionBuffer;
	std::unique_ptr<GLBuffer> m_pTexcoordBuffer;
	std::unique_ptr<GLBuffer> m_pIndexBuffer;
};

#endif RENDER_TEXTURE_NODE_H
#ifndef POST_EFFECT_H
#define POST_EFFECT_H
#include "RenderTarget.h"
#include "Texture.h"
#include "IShader.h"
#include "RenderNode.h"
namespace KI
{

class IPostEffect
{
public:
	IPostEffect() {};
	~IPostEffect() {};

private:

};


class TextureDrawer
{
public:
	TextureDrawer() {};
	~TextureDrawer() {};

	static void Execute(const DrawContext& context, const Texture* pTexture);
private:

};

class RenderTextureNode
{
public:
	RenderTextureNode();
	~RenderTextureNode() {};

	const Unique<GLBuffer>& GetPositionBuffer() const { return m_pPositionBuffer; }
	const Unique<GLBuffer>& GetTexcoordBuffer() const { return m_pTexcoordBuffer; }
	const Unique<GLBuffer>& GetIndexBuffer() const { return m_pIndexBuffer; }
private:
	void BuildGLBuffer();
	Unique<GLBuffer> m_pPositionBuffer;
	Unique<GLBuffer> m_pTexcoordBuffer;
	Unique<GLBuffer> m_pIndexBuffer;
};

class ComputeTextureCombiner : public IPostEffectShader
{
public:
	ComputeTextureCombiner() {};
	~ComputeTextureCombiner() {};

	void Execute(const DrawContext& context, RenderTarget& target);
	virtual ShaderPath GetShaderPath();
	virtual void FetchUniformLocation();
private:
	GLuint m_uColorForward;
	GLuint m_uDepthForward;
	GLuint m_uColorCompute;
	GLuint m_uDepthCompute;
};


}
#endif POST_EFFECT_H
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

	void Execute(const DrawContext& context);
	virtual ShaderPath GetShaderPath();
	virtual void FetchUniformLocation();
private:
	GLuint m_uColorForward;
	GLuint m_uDepthForward;
	GLuint m_uColorCompute;
	GLuint m_uDepthCompute;
	GLuint m_uImageSize;
};

class EdgeDetector : public IPostEffectShader
{
public:
	EdgeDetector()
		: m_diffNormal(0.0f)
		, m_diffDepth(0.0f)	{};
	~EdgeDetector() {};

	void Execute(const DrawContext& context);
	virtual ShaderPath GetShaderPath();
	virtual void FetchUniformLocation();
	void SetDiffNormal(float value) { m_diffNormal = value; }
	void SetDiffDepth(float value) { m_diffDepth = value; }
private:
	GLuint m_uColorTexture;
	GLuint m_uNormalTexture;
	GLuint m_uDepthTexture;
	GLuint m_uTexelSize;
	GLuint m_uDiffDepth;
	GLuint m_uDiffNormal;
	float m_diffNormal;
	float m_diffDepth;
};

class PostEffect : public IPostEffect
{
public:
	PostEffect() {};
	~PostEffect() {};

	void Build();
	void Execute(DrawContext& context);
	void ShowUI(UIContext& context);

private:
	struct UI
	{
		UI()
			: edgeDetector(false)
			, diffDepth(0.001f)
			, diffNormal(0.1f)
		{
		}
		bool edgeDetector;
		float diffDepth;
		float diffNormal;
	};

	UI m_ui;
	EdgeDetector m_EdgeDetector;
};

}
#endif POST_EFFECT_H
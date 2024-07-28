#ifndef SKYBOX_NODE_H
#define SKYBOX_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Texture.h"
#include "Primitives.h"
namespace KI
{
class SkyBoxNode : public RenderNode
{
public:

	class Shader : public IShadingShader
	{
	public:
		Shader();
		~Shader();
		enum UNIFORM
		{
			MODEL,
			SKYBOX,
			NUM
		};

		virtual ShaderPath GetShaderPath();
		virtual void GetUniformLocation();

		void SetCamera(const GLBuffer* pBuffer);
		void SetModel(const Matrix4x4& value) override;
		void SetPosition(GLBuffer* pPosition);
		void SetTexture(Texture* pTexure);
		void SetTexcoord(GLBuffer* pTexture);
	private:
		GLuint m_uniform[UNIFORM::NUM];
	};



	SkyBoxNode();
	~SkyBoxNode();

	virtual void Draw(const DrawContext& context);
	Vector<String> GetTexturePath();
private:
	void BuildGLBuffer();

	Unique<SkyBox> m_skybox;
	Unique<GLBuffer> m_pPositionBuffer;
	Unique<CubemapTexture> m_pCubemap;;
	Unique<SkyBoxNode::Shader> m_pShader;
};


}
#endif SKYBOX_NODE_H

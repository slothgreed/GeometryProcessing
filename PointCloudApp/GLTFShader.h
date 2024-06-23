#ifndef GLTF_SHADER_H
#define GLTF_SHADER_H
#include "IShader.h"
namespace KI
{
struct GLTFMaterial;
struct GLTFPrimitive;
class GLTFShader : public IShadingShader
{
public:
	GLTFShader() {};
	virtual ~GLTFShader() {};

	enum UNIFORM
	{
		MODEL,
		SSBO_INDEX,
		COLOR_TEXTURE,
		NORMAL_TEXTURE,
		ROUGHNESS_TEXTURE,
		NUM
	};

	virtual ShaderPath GetShaderPath() override;
	void SetVertexBuffer(GLBuffer* pBuffer, const VertexFormats& format);
	void SetModel(const Matrix4x4& value);
	void SetSkinBuffer(const GLBuffer* pBuffer);
	void SetIndexBuffer(GLBuffer* pBuffer);
	void SetNodeBuffer(const GLBuffer* pBuffer);
	void SetMaterialBuffer(const GLBuffer* pBuffer);
	void BindBufferIndex(int matrix, int material);
	virtual void GetUniformLocation();
	virtual void SetCamera(const GLBuffer* pBuffer);
	void DrawElement(const GLTFPrimitive& primitive, GLuint dataType);
	void BindBaseColor(const Texture& texture);
	void BindNormal(const Texture& texture);
	void BindRoughness(const Texture& texture);

private:
	GLuint m_uniform[UNIFORM::NUM];
};

}

#endif GLTF_SHADER_H
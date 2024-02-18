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
	~GLTFShader() {};

	enum UNIFORM
	{
		VIEW_PROJ,
		MODEL,
		COLOR_TEXTURE,
		NORMAL_TEXTURE,
		ROUGHNESS_TEXTURE,
		NUM
	};

	virtual IShadingShader::Type GetType() { return IShadingShader::Type::GLTF; }
	virtual String GetVertexPath() override;
	virtual String GetFragmentPath() override;
	void SetVertexBuffer(GLBuffer* pBuffer, const VertexFormats& format);
	void SetIndexBuffer(GLBuffer* pBuffer);

	void BindMaterial(const GLTFMaterial& material);
	virtual void GetUniformLocation();
	virtual void SetViewProj(const Matrix4x4& value);
	void SetModel(const Matrix4x4& value);
	void DrawElement(const GLTFPrimitive& primitive, GLuint dataType);

private:
	GLuint m_uniform[UNIFORM::NUM];
	void BindBaseColor(const Texture& texture);
	void BindNormal(const Texture& texture);
	void BindRoughness(const Texture& texture);
};

}

#endif GLTF_SHADER_H
#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H
#include "IShader.h"
#include "Texture.h"
namespace KI
{

// MEMO : FaceShaderÇ™åpè≥ÇµÇƒÇ¢ÇÈÅB
class SimpleShader : public IShadingShader
{
public:
	SimpleShader();
	~SimpleShader();

	enum UNIFORM
	{
		MODEL,
		COLOR,
		NUM
	};

	virtual ShaderPath GetShaderPath() override;
	
	void FetchUniformLocation();
	void SetCamera(const GLBuffer* pBuffer);
	void SetModel(const Matrix4x4& value);
	void SetColor(const Vector3& value);
	void SetPosition(GLBuffer* pBuffer);
private:

	GLuint m_uniform[UNIFORM::NUM];

};

class FaceShader : public SimpleShader
{
public:
	FaceShader() {};
	~FaceShader() {};

	void SetNormal(GLBuffer* pBuffer);
	virtual int GetDrawTargetNum() const { return 3; }
	virtual ShaderPath GetShaderPath() override;
private:
};
class VertexColorShader : public IShadingShader
{
public:
	VertexColorShader();
	~VertexColorShader();

	enum UNIFORM
	{
		MODEL,
		NUM
	};

	virtual ShaderPath GetShaderPath() override;

	virtual void FetchUniformLocation() override;
	virtual void SetCamera(const GLBuffer* pBuffer);
	virtual void SetModel(const Matrix4x4& value) override;
	void SetPosition(GLBuffer* pPosition);
	void SetColor(GLBuffer* pColor);

private:

	GLuint m_uniform[UNIFORM::NUM];

};

class PrimitiveColorShader : public IShadingShader
{
public:
	enum UNIFORM
	{
		MODEL,
		NUM
	};
	PrimitiveColorShader() {};
	~PrimitiveColorShader() {};
	virtual ShaderPath GetShaderPath() override;
	virtual void FetchUniformLocation() override;
	virtual void SetCamera(const GLBuffer* pBuffer);
	virtual void SetModel(const Matrix4x4& value) override;
	void SetPosition(GLBuffer* pPosition);
	void SetColor(GLBuffer* pColor);

private:
	GLuint m_uniform[UNIFORM::NUM];

};

class InstancedPrimitiveShader : public IShadingShader
{
public:
	enum UNIFORM
	{
		COLOR,
		NUM
	};

	InstancedPrimitiveShader() {};
	~InstancedPrimitiveShader() {};
	virtual ShaderPath GetShaderPath() override;
	virtual void FetchUniformLocation() override;
	virtual void SetCamera(const GLBuffer* pBuffer);
	void SetPosition(const GLBuffer* pPosition);
	void SetColor(const Vector3& color);
	void SetMatrixTexture(const TextureBuffer* pBuffer);
private:
	GLuint m_uniform[UNIFORM::NUM];
};

class VertexVectorShader : public IShadingShader
{
public:
	VertexVectorShader() {};
	~VertexVectorShader() {};

	enum UNIFORM
	{
		COLOR,
		LENGTH,
		MODEL,
		NUM
	};

	virtual ShaderPath GetShaderPath() override;
	virtual void FetchUniformLocation() override;
	virtual void SetCamera(const GLBuffer* pBuffer);
	void SetPosition(const GLBuffer* pPosition);
	void SetVector(const GLBuffer* pVector);
	void SetModel(const Matrix4x4& value);
	void SetColor(const Vector4& color);
	void SetLength(float length);
private:
	GLuint m_uniform[UNIFORM::NUM];
};

class PointPickShader : public IShadingShader
{
public:

	enum IDType
	{
		PrimitiveID,
		PickID,
	};

	PointPickShader(IDType type):m_type(type) {};
	~PointPickShader() {};

	enum UNIFORM
	{
		MODEL,
		PICKOFFSET,
		NUM
	};

	virtual ShaderPath GetShaderPath() override;
	virtual void FetchUniformLocation() override;
	virtual void SetCamera(const GLBuffer* pBuffer);
	void SetPosition(const GLBuffer* pPosition);
	void SetPosition(VertexFormat format, const GLBuffer* pPosition);
	void SetModel(const Matrix4x4& value);
	void SetPickOffset(unsigned int offset);
	void SetPickID(unsigned int pickID);
private:
	IDType m_type;
	GLuint m_uniform[UNIFORM::NUM];

};

class Texture;
class TextureShader : public IShadingShader
{
public:

	enum Type
	{
		UINT,
		VEC4
	};

	enum UNIFORM
	{
		TEXTURE,
		MODEL,
		NUM
	};

	TextureShader(Type type)
		: m_type(type)
	{
	};
	virtual ~TextureShader() {};
	virtual ShaderPath GetShaderPath();
	virtual void FetchUniformLocation();
	virtual void SetCamera(const GLBuffer* pBuffer);
	void SetModel(const Matrix4x4& value);
	void BindTexture(const Texture& texture);
	void SetPosition(GLBuffer* pPosition);
	void SetTexcoord(GLBuffer* pTexture);
private:
	Type m_type;
	GLuint m_uniform[UNIFORM::NUM];
};


class TextureViewShader : public IPostEffectShader
{
public:

	TextureViewShader()
		: m_uTexture(0) {};
	virtual ~TextureViewShader() {};
	virtual ShaderPath GetShaderPath();
	virtual void FetchUniformLocation();
	void BindTexture(const Texture& texture);
private:
	GLuint m_uTexture;
};

class CubemapViewShader : public IPostEffectShader
{
public:

	CubemapViewShader() {};
	virtual ~CubemapViewShader() {};
	virtual ShaderPath GetShaderPath();
	virtual void FetchUniformLocation();
	void BindMipmapLevel(int level);
	void BindTexture(const CubemapTexture& texture);
private:
	GLuint m_uMipmap;
	GLuint m_uCubeMap;
};

}
#endif SIMPLE_SHADER_H
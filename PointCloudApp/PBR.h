#ifndef PBR_TEXTURE_H
#define	PBR_TEXTURE_H
#include "Texture.h"
#include "IShader.h"
namespace KI
{
class PBRTextureGenerator
{
public:
	PBRTextureGenerator() {};
	~PBRTextureGenerator() {};

	static Texture* GenerateBRDFLUT(const Vector2i& resolute);
	static CubemapTexture* GenerateIrradianceMap(const Vector2i& resolute, const CubemapTexture* pCubemap);
	static CubemapTexture* GeneratePrefilteredMap(const Vector2i& resolute, const CubemapTexture* pCubemap);
private:

	struct BRDFLUTShader : public IComputeShader
	{
		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation() {};
	};

	struct IrradiaceMapShader : public IComputeShader
	{
		virtual Vector3i GetLocalThreadNum() const { return Vector3i(8, 8, 6); }
		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();
		void BindEnvironment(const CubemapTexture& cubeMap);
		GLuint m_EnvironmentMap;
	};


	struct PrefilteredMapShader : public IComputeShader
	{
		virtual Vector3i GetLocalThreadNum() const { return Vector3i(8, 8, 6); }
		virtual ShaderPath GetShaderPath();
		virtual void FetchUniformLocation();
		void BindEnvironment(const CubemapTexture& cubeMap);
		void BindRoughness(float roughness);
		GLuint m_EnvironmentMap;
		GLuint m_Roughness;

	};
};

class PBRResource
{
public:
	PBRResource()
		:m_pGlobal(nullptr)
	{
	};
	~PBRResource();

	void Initialize(const CubemapTexture& global);
	const GLBuffer* GetGlobalParam() const { return m_pGlobal; }
	const Texture* GetBRDFLUT() const { return m_pBRDFLUT.get(); }
	const CubemapTexture* GetIrradiance() const { return m_pIrradiance.get(); }
	const CubemapTexture* GetPrefiltered() const { return m_pPrefiltered.get(); }
	void Update();
private:
	GLBuffer* m_pGlobal;
	Unique<Texture> m_pBRDFLUT;
	Unique<CubemapTexture> m_pIrradiance;
	Unique<CubemapTexture> m_pPrefiltered;
};


}


#endif // PBR_TEXTURE_H
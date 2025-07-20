#include "PBR.h"

namespace KI
{

Texture* PBRTextureGenerator::GenerateBRDFLUT(const Vector2i& resolute)
{
	auto pTexture = new Texture2D();
	pTexture->Build(resolute.x, resolute.y);
	auto pShader = std::make_unique<BRDFLUTShader>();
	pShader->Build();
	pShader->Use();
	pShader->BindTexture(0, pTexture, GL_WRITE_ONLY);
	pShader->Dispatch(resolute.x, resolute.y, 1);
	pShader->BarrierImage();

	return pTexture;

}
CubemapTexture* PBRTextureGenerator::GenerateIrradianceMap(const Vector2i& resolute, const CubemapTexture* pCubemap)
{
	auto pIrradianceMapArray = std::make_unique<CubemapTexture>();
	pIrradianceMapArray->BuildArray(resolute, false);
	auto pShader = std::make_unique<IrradiaceMapShader>();
	assert(resolute.x % pShader->GetLocalThreadNum().x == 0);
	assert(resolute.y % pShader->GetLocalThreadNum().y == 0);
	pShader->Build();
	pShader->Use();
	pShader->BindEnvironment(*pCubemap);
	pShader->BindTexture(1, pIrradianceMapArray.get(), GL_WRITE_ONLY);
	pShader->Dispatch(pShader->GetDispatchNum2D(resolute));
	pShader->BarrierImage();

	return CubemapTexture::ConvertCubemap(pIrradianceMapArray.get());
}

CubemapTexture* PBRTextureGenerator::GeneratePrefilteredMap(const Vector2i& resolute, const CubemapTexture* pCubemap)
{
	auto pPrefilteredArray = std::make_unique<CubemapTexture>();
	pPrefilteredArray->BuildArray(resolute, true);
	auto pShader = std::make_unique<PrefilteredMapShader>();
	assert(resolute.x % pShader->GetLocalThreadNum().x == 0);
	assert(resolute.y % pShader->GetLocalThreadNum().y == 0);
	pShader->Build();
	pShader->Use();
	pShader->BindEnvironment(*pCubemap);
	for (int mipmap = 0; mipmap < pPrefilteredArray->GetFormat().level; mipmap++) {
		pShader->BindRoughness((float)mipmap / (pPrefilteredArray->GetFormat().level - 1));
		pShader->BindTexture(1, mipmap, pPrefilteredArray.get(), GL_WRITE_ONLY);
		pShader->Dispatch(pShader->GetDispatchNum2D(Texture2D::CalcMipmapResolute(resolute, mipmap)));
		pShader->BarrierImage();
	}

	return CubemapTexture::ConvertCubemap(pPrefilteredArray.get());
}


ShaderPath PBRTextureGenerator::BRDFLUTShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "pbr\\BRDFLUT.comp";
	return path;
}

ShaderPath PBRTextureGenerator::IrradiaceMapShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "pbr\\irradianceMap.comp";
	return path;
}

void PBRTextureGenerator::IrradiaceMapShader::BindEnvironment(const CubemapTexture& cubeMap)
{
	glActiveTexture(GL_TEXTURE0);
	OUTPUT_GLERROR;
	glUniform1i(m_EnvironmentMap, 0);
	OUTPUT_GLERROR;
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.Handle());
	OUTPUT_GLERROR;
}

void PBRTextureGenerator::IrradiaceMapShader::FetchUniformLocation()
{
	m_EnvironmentMap = GetUniformLocation("u_EnvirionmentMap");
}


ShaderPath PBRTextureGenerator::PrefilteredMapShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "pbr\\PrefilteredMap.comp";
	return path;
}

void PBRTextureGenerator::PrefilteredMapShader::BindEnvironment(const CubemapTexture& cubeMap)
{
	glActiveTexture(GL_TEXTURE0);
	OUTPUT_GLERROR;
	glUniform1i(m_EnvironmentMap, 0);
	OUTPUT_GLERROR;
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.Handle());
	OUTPUT_GLERROR;
}

void PBRTextureGenerator::PrefilteredMapShader::BindRoughness(float roughness)
{
	BindUniform(m_Roughness, roughness);
}

void PBRTextureGenerator::PrefilteredMapShader::FetchUniformLocation()
{
	m_EnvironmentMap = GetUniformLocation("u_EnvirionmentMap");
	m_Roughness = GetUniformLocation("u_Roughness");
}


void PBRResource::Initialize(const CubemapTexture& global)
{
	m_pBRDFLUT = std::unique_ptr<Texture>(PBRTextureGenerator::GenerateBRDFLUT(Vector2i(512, 512)));
	m_pIrradiance = std::unique_ptr<CubemapTexture>(PBRTextureGenerator::GenerateIrradianceMap(Vector2i(64, 64), &global));
	m_pPrefiltered = std::unique_ptr<CubemapTexture>(PBRTextureGenerator::GeneratePrefilteredMap(Vector2i(512, 512), &global));

}


}

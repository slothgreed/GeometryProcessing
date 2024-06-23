#include "ComputeShaderSample.h"
#include "Texture.h"
namespace KI
{



ShaderPath CreateTextureComputeShader::GetShaderPath()
{
	ShaderPath path;
	path.shader[SHADER_PROGRAM_COMPUTE] = "sample\\texture.comp";
	return path;
}
void CreateTextureComputeShader::Execute(const Shared<Texture>& pTexture)
{
	pTexture->Bind();
	Use();
	OUTPUT_GLERROR;
	glBindImageTexture(0, pTexture->Handle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	OUTPUT_GLERROR;
	glDispatchCompute(pTexture->Size().x, pTexture->Size().y, 1);
	OUTPUT_GLERROR;
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	OUTPUT_GLERROR;
	UnUse();
}

void CreateTextureComputeShader::GetUniformLocation()
{

}
}
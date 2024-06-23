#ifndef COMPUTE_SHADER_SAMPLE_H
#define COMPUTE_SHADER_SAMPLE_H
#include "IShader.h"
namespace KI
{



class CreateTextureComputeShader : public IComputeShader
{
public:
	CreateTextureComputeShader() {};
	~CreateTextureComputeShader() {};

	virtual ShaderPath GetShaderPath() override;
	void Execute(const Shared<Texture>& pTexture);
	virtual void GetUniformLocation() override;
private:

};
}

#endif COMPUTE_SHADER_H
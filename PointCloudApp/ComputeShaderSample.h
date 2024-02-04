#ifndef COMPUTE_SHADER_SAMPLE_H
#define COMPUTE_SHADER_SAMPLE_H
#include "IShader.h"

class SimpleComputeShader : public IComputeShader
{
public:
	SimpleComputeShader();
	~SimpleComputeShader();

	virtual std::string GetComputePath() override;

	void Execute(const std::vector<float>& in, std::vector<float>& out);
	virtual void GetUniformLocation() override;
private:
	int m_elementSize;
	GLuint m_uniform;
};

class CreateTextureComputeShader : public IComputeShader
{
public:
	CreateTextureComputeShader() {};
	~CreateTextureComputeShader() {};

	virtual std::string GetComputePath() override;
	void Execute(const std::shared_ptr<Texture>& pTexture);
	virtual void GetUniformLocation() override;
private:

};


#endif COMPUTE_SHADER_H
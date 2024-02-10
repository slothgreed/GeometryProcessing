#include "ComputeShaderSample.h"
#include "Texture.h"
SimpleComputeShader::SimpleComputeShader()
{
	m_elementSize = 0;
	m_dimension = ivec3(256, 1, 1);
}

SimpleComputeShader::~SimpleComputeShader()
{
	Delete();
}

String SimpleComputeShader::GetComputePath()
{
	return  "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\ComputeShader\\simple.comp";
}

void SimpleComputeShader::Execute(const Vector<float>& in, Vector<float>& out)
{
	auto buffer = std::make_unique<GLBuffer>();
	buffer->Create(in);

	Use();

	glUniform1ui(m_uniform, in.size());

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffer->Handle());
	glDispatchCompute(m_elementSize / 256 + 1, m_dimension.y, m_dimension.z);
	UnUse();

	out.resize(in.size());
	buffer->GetBufferData(out);
}

void SimpleComputeShader::GetUniformLocation()
{
	m_uniform = glGetUniformLocation(m_programId, "u_elementSize");
}

String CreateTextureComputeShader::GetComputePath()
{
	return  "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\ComputeShader\\texture.comp";
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
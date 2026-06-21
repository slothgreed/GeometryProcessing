#include <iostream>
#include "ComputeShaderTest.h"
#include "GLBuffer.h"
#include <random>

namespace KI
{

void ComputeShaderTest::Initialize()
{
	if (glfwInit() == GL_FALSE) {
		std::cerr << "Can't initilize GLFW" << std::endl;
		return;
	}

	GLFWwindow* window = glfwCreateWindow(1024, 768, "PointCloudApp", NULL, NULL);
	if (window == NULL) {
		return;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		return;
	}
}

void ComputeShaderTest::Finalize()
{
	glfwTerminate();
}
Vector<uint32_t> ComputeShaderTest::CreateData(int pattern, int num)
{
	std::vector<uint32_t> result;
	result.reserve(num);

	std::mt19937 rng(12345); // ÉeÉXÉgçƒåªê´ÇÃÇΩÇﬂå≈íËseed

	uint32_t minValue = 0;
	uint32_t maxValue = 0;

	switch (pattern) {
	case 1:		// 0 Å` 7bit
		maxValue = (1u << 8) - 1;
		break;

	case 2:		// 0 Å` 15bit
		maxValue = (1u << 16) - 1;
		break;

	case 3:		// 0 Å` 23bit
		maxValue = (1u << 24) - 1;
		break;

	case 4:		// 0 Å` 31bit
		maxValue = 0xFFFFFFFFu; //(1u << 32) - 1;
		break;

	default:
		return result;
	}

	const std::vector<uint32_t> boundaries =
	{
		0u,
		1u,
		0x0000007Fu,
		0x00000080u,
		0x000000FFu,
		0x00000100u,
		0x00007FFFu,
		0x00008000u,
		0x0000FFFFu,
		0x00010000u,
		0x007FFFFFu,
		0x00800000u,
		0x00FFFFFFu,
		0x01000000u,
		0x7FFFFFFFu,
		0x80000000u,
		0xFFFFFFFFu
	};

	result.reserve(num);

	for (uint32_t v : boundaries) {
		if (result.size() >= static_cast<size_t>(num)) break;
		if (v <= maxValue) {
			result.push_back(v);
		}
	}

	std::uniform_int_distribution<uint32_t> dist(minValue, maxValue);

	for (int i = 0; i < num; ++i) {
		result.push_back(dist(rng));
	}

	return result;
}
void PrefixSumTest::Execute()
{	
	auto in = CreateData(3, 10000);
	auto outCPU = ExecuteCPU(in);
	auto outGPU = ExecuteGPU(in);

	if (Validate(outCPU, outGPU)) {
		std::cout << "Success" << std::endl;
	} else {
		std::cout << "Failed" << std::endl;
	}
}

bool PrefixSumTest::Validate(const Vector<uint32_t>& in, const Vector<uint32_t>& out)
{
	return in == out;
}

Vector<uint32_t> PrefixSumTest::ExecuteCPU(const Vector<uint32_t>& in)
{
	Vector<uint32_t> out(in.size());

	uint32_t sum = 0;
	for (size_t i = 0; i < in.size(); ++i) {
		out[i] = sum;
		sum += in[i];
	}

	return out;
}
Vector<uint32_t> PrefixSumTest::ExecuteGPU(const Vector<uint32_t>& in)
{
	auto shader = std::make_unique<PrefixSumShader>();
	shader->Build();

	return shader->Execute(in);
}

PrefixSumShader::PrefixSumShader()
	: m_pPost(std::make_unique<PrefixSumShaderPost>())
{
	m_pPost->Build();
}
ShaderPath PrefixSumShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "sample\\prefixsum.comp";
	return path;
}

void PrefixSumShader::FetchUniformLocation()
{
	m_num = GetUniformLocation("u_num");
	m_localPhase = GetUniformLocation("u_localPhase");
}

void PrefixSumShaderPost::Execute(Unique<GLBuffer>& outBuffer, Unique<GLBuffer>& blockSum)
{
	Use();
	BindUniform(m_num, outBuffer->Num());
	BindShaderStorage(0, outBuffer->Handle());
	BindShaderStorage(1, blockSum->Handle());
	Dispatch(GetDispatchNum1D(outBuffer->Num()));
	BarrierSSBO();
	UnUse();
}


void PrefixSumShader::Execute(const Unique<GLBuffer>& inBuffer, Unique<GLBuffer>& outBuffer)
{
	auto blockSumBuffer = std::make_unique<GLBuffer>();
	Vector<uint32_t> blockSum(GetDispatchNum1D(inBuffer->Num()).x, 0);
	blockSumBuffer->Create(blockSum);

	Use();
	BindUniform(m_localPhase, (int)1);
	BindUniform(m_num, (int)inBuffer->Num());
	BindShaderStorage(0, inBuffer->Handle());
	BindShaderStorage(1, outBuffer->Handle());
	BindShaderStorage(2, blockSumBuffer->Handle());
	Dispatch(GetDispatchNum1D(inBuffer->Num()));
	BarrierSSBO();
	if (inBuffer->Num() < 1024) {
		UnUse();
		return;
	}


	auto outBlockSumBuffer = std::make_unique<GLBuffer>();
	Vector<uint32_t> outBlockSum(blockSum.size());
	outBlockSumBuffer->Create(outBlockSum);
	BindUniform(m_localPhase, (int)0);
	BindUniform(m_num, (int)blockSum.size());
	BindShaderStorage(0, blockSumBuffer->Handle());
	BindShaderStorage(1, outBlockSumBuffer->Handle());
	Dispatch(GetDispatchNum1D(blockSum.size()));
	BarrierSSBO();

	blockSumBuffer->GetBufferData(blockSum);
	outBlockSumBuffer->GetBufferData(outBlockSum);


	m_pPost->Execute(outBuffer, outBlockSumBuffer);
	return;
}
Vector<uint32_t> PrefixSumShader::Execute(const Vector<uint32_t>& in)
{
	Vector<uint32_t> out(in.size());
	auto buffer = std::make_unique<GLBuffer>();
	buffer->Create(in);
	auto outBuffer = std::make_unique<GLBuffer>();
	outBuffer->Create(out);
	auto blockSumBuffer = std::make_unique<GLBuffer>();
	Vector<uint32_t> blockSum(GetDispatchNum1D(in.size()).x, 0);
	blockSumBuffer->Create(blockSum);
	
	Use();
	BindUniform(m_localPhase, (int)1);
	BindUniform(m_num, (int)in.size());
	BindShaderStorage(0, buffer->Handle());
	BindShaderStorage(1, outBuffer->Handle());
	BindShaderStorage(2, blockSumBuffer->Handle());
	Dispatch(GetDispatchNum1D(in.size()));
	BarrierSSBO();
	if (in.size() < 1024) {
		outBuffer->GetBufferData(out);
		return out;
	}


	auto outBlockSumBuffer = std::make_unique<GLBuffer>();
	Vector<uint32_t> outBlockSum(blockSum.size());
	outBlockSumBuffer->Create(outBlockSum);
	BindUniform(m_localPhase, (int)0);
	BindUniform(m_num, (int)blockSum.size());
	BindShaderStorage(0, blockSumBuffer->Handle());
	BindShaderStorage(1, outBlockSumBuffer->Handle());
	Dispatch(GetDispatchNum1D(blockSum.size()));
	BarrierSSBO();

	blockSumBuffer->GetBufferData(blockSum);
	outBlockSumBuffer->GetBufferData(outBlockSum);


	m_pPost->Execute(outBuffer, outBlockSumBuffer);
	outBuffer->GetBufferData(out);
	return out;
}


ShaderPath PrefixSumShaderPost::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "sample\\prefixsum_post.comp";
	return path;
}

void PrefixSumShaderPost::FetchUniformLocation()
{
	m_num = GetUniformLocation("u_num");
}




void RadixSortTest::Execute()
{	
	auto in = CreateData(1, 40);
	auto outCPU = ExecuteCPU(in);
	auto outGPU = ExecuteGPU(in);

	if(Validate(in, outCPU) && Validate(in, outGPU)) {
		std::cout << "Success" << std::endl;
	} else {
		std::cout << "Failed" << std::endl;
	}
}

Vector<uint32_t> RadixSortTest::ExecuteCPU(const Vector<uint32_t>& in)
{
	if (in.empty()) return Vector<uint32_t>();

	auto out = in;
	auto temp = in;
	for (size_t shift = 0; shift < 32; shift += 8) {
		std::array<size_t, 256> offset = {};
		for (auto value : out) {
			offset[(value >> shift) & 255]++;
		}

		size_t sum = 0;
		for (size_t i = 0; i < 256; i++) {
			size_t c = offset[i];
			offset[i] = sum;
			sum += c;
		}

		for (auto value : out) {
			uint32_t digit = (value >> shift) & 255;
			temp[offset[digit]] = value;
			offset[digit]++;
		}

		out.swap(temp);
	}

	return out;
}
Vector<uint32_t> RadixSortTest::ExecuteGPU(const Vector<uint32_t>& in)
{
	auto pHistogramShader = std::make_unique<HistogramShader>(); pHistogramShader->Build();
	auto pPrefixSumShader = std::make_unique<PrefixSumShader>(); pPrefixSumShader->Build();
	auto pPrefixSumShaderPost = std::make_unique<PrefixSumShaderPost>(); pPrefixSumShaderPost->Build();
	auto pScatterShader = std::make_unique<ScatterShader>(); pScatterShader->Build();
	auto pInBuffer = std::make_unique<GLBuffer>(); pInBuffer->Create(in);
	auto pOutBuffer = std::make_unique<GLBuffer>(); pOutBuffer->Create(in);
	auto pHistogramBuffer = std::make_unique<GLBuffer>(); pHistogramBuffer->Create(256, sizeof(uint32_t));
	auto pPrefixSumBuffer = std::make_unique<GLBuffer>(); pPrefixSumBuffer->Create(256, sizeof(uint32_t));
	for (uint shift = 0; shift < 32; shift += 8) {
		pHistogramBuffer->SetData(0);
		pHistogramShader->Execute(pInBuffer, pHistogramBuffer, shift, 256);
		pPrefixSumShader->Execute(pHistogramBuffer, pPrefixSumBuffer);
		pScatterShader->Execute(pInBuffer, pOutBuffer, pPrefixSumBuffer, shift);
		std::swap(pInBuffer, pOutBuffer);
	}

	Vector<uint32_t> result(in.size());
	pInBuffer->GetBufferData(result);
	return result;
}
bool RadixSortTest::Validate(const Vector<uint32_t>& in, const Vector<uint32_t>& out)
{
	auto sorted = in;
	std::sort(sorted.begin(), sorted.end());
	return sorted == out;
}

ShaderPath RadixSortTest::ScatterShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "sample\\radixsort_scatter.comp";
	return path;
}


void RadixSortTest::ScatterShader::FetchUniformLocation()
{
	m_num = GetUniformLocation("u_num");
	m_uBitShift = GetUniformLocation("u_bitShift");
}
void RadixSortTest::ScatterShader::Execute(const Unique<GLBuffer>& in, const Unique<GLBuffer>& out, const Unique<GLBuffer>& offset, uint bitShift)
{
	Use();
	BindUniform(m_num, (uint)in->Num());
	BindUniform(m_uBitShift, bitShift);
	BindShaderStorage(0, in->Handle());
	BindShaderStorage(1, out->Handle());
	BindShaderStorage(2, offset->Handle());
	Dispatch(GetDispatchNum1D(in->Num()));
	BarrierSSBO();
	UnUse();
}
void HistogramTest::Execute()
{	
	std::vector<uint32_t> input;
	input.resize(100000);

	std::mt19937 rng(12345); // ÉeÉXÉgçƒåªê´ÇÃÇΩÇﬂå≈íËseed

	uint32_t minValue = 0;
	uint32_t maxValue = (1u << 8) - 1;

	std::uniform_int_distribution<uint32_t> dist(minValue, maxValue);

	for (int i = 0; i < input.size(); ++i) {
		input[i] = dist(rng);
	}

	auto shader = std::make_unique<HistogramShader>();
	shader->Build();
	std::vector<uint32_t> gpu;
	gpu = shader->Execute(input, 0, 256);

	std::vector<uint32_t> cpu(256);
	for (auto value : input) {
		cpu[(value >> 0) & 255]++;
	}

	if(cpu != gpu) {
		std::cout << "Failed" << std::endl;
	} else {
		std::cout << "Success" << std::endl;
	}

}


void HistogramShader::FetchUniformLocation()
{
	m_num = GetUniformLocation("u_num");
	m_bitShift = GetUniformLocation("u_bitShift");
	m_binNum = GetUniformLocation("u_binNum");
}

ShaderPath HistogramShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.shader[SHADER_PROGRAM_COMPUTE] = "sample\\radixsort_histogram.comp";
	return path;
}

Vector3i HistogramShader::GetLocalThreadNum() const
{
	return Vector3i(1024, 1, 1);
}

void HistogramShader::Execute(const Unique<GLBuffer>& in, Unique<GLBuffer>& out, uint bitShift, uint binNum)
{
	Use();
	BindUniform(m_num, (uint)in->Num());
	BindUniform(m_bitShift, bitShift);
	BindUniform(m_binNum, binNum);
	BindShaderStorage(0, in->Handle());
	BindShaderStorage(1, out->Handle());
	Dispatch(GetDispatchNum1D(in->Num()));
	BarrierSSBO();
	UnUse();
}

Vector<uint32_t> HistogramShader::Execute(const Vector<uint32_t>& in, uint bitShift, uint binNum)
{
	auto buffer = std::make_unique<GLBuffer>();
	buffer->Create(in);
	auto outBuffer = std::make_unique<GLBuffer>();
	Vector<uint32_t> out;
	out.resize(binNum);

	outBuffer->Create(out);
	Use();
	BindUniform(m_num, (uint)in.size());
	BindUniform(m_bitShift, (uint)0);
	BindUniform(m_binNum, (uint)binNum);
	BindShaderStorage(0, buffer->Handle());
	BindShaderStorage(1, outBuffer->Handle());
	Dispatch(GetDispatchNum1D(in.size()));
	BarrierSSBO();
	UnUse();
	outBuffer->GetBufferData(out);
	
	return out;	
}
}
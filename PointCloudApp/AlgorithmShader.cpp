#include "AlgorithmShader.h"
#include "Voxel.h"
namespace KI
{

void MarchingCubeShader::Build()
{
	IMeshShader::Build();
	MarchingCube marching;
	pTriTable = std::make_unique<GLBuffer>();
	pTriTable->Create(marching.CreateFlattenTriangleTable());

	if (m_withTask) {
		Vector<Vector2> value(m_voxelSize, Vector2(0));
		pCubeIndexs = std::make_unique<GLBuffer>();
		pCubeIndexs->Create(value);
		Vector<UInt> taskNum(CeilDiv(m_voxelSize, GetTaskThreadNum()), 0);
		pTaskNums = std::make_unique<GLBuffer>();
		pTaskNums->Create(taskNum);
	}
}

ShaderPath MarchingCubeShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	if (m_dataType == DATA_FLOAT) {
		path.define.push_back("#define VOXEL_DATA_TYPE_FLOAT");
	}
	path.header.push_back("common.h");
	path.header.push_back("volume\\voxel.h");
	path.header.push_back("volume\\marchingCube.h");

	if (m_withTask) {
		path.shader[SHADER_PROGRAM_TASK] = "volume\\marchingCube.task";
		path.shader[SHADER_PROGRAM_MESH] = "volume\\marchingCubeWithTask.mesh";
	} else {
		path.shader[SHADER_PROGRAM_MESH] = "volume\\marchingCube.mesh";
	}
	path.shader[SHADER_PROGRAM_FRAG] = "volume\\marchingCube.frag";
	return path;
}

void MarchingCubeShader::FetchUniformLocation()
{
	m_uModel = GetUniformLocation("u_Model");
	m_uThreshold = GetUniformLocation("u_threshold");
	m_uTaskNum = GetUniformLocation("u_taskNum");
}

void MarchingCubeShader::SetCamera(const GLBuffer* pBuffer)
{
	BindShaderStorage(0, pBuffer->Handle());
}
void MarchingCubeShader::SetVoxel(const GLBuffer* pBuffer)
{
	BindShaderStorage(1, pBuffer->Handle());
}
void MarchingCubeShader::SetVoxelData(const GLBuffer* pBuffer)
{
	BindShaderStorage(2, pBuffer->Handle());
}
void MarchingCubeShader::SetModel(const Matrix4x4& value)
{
	BindUniform(m_uModel, value);
}
void MarchingCubeShader::SetThreshold(float threshold)
{
	BindUniform(m_uThreshold, threshold);
}

void MarchingCubeShader::PreDraw()
{
	BindShaderStorage(3, pTriTable->Handle());
	if (m_withTask) {
		pTaskNums->SetData(0);
		pCubeIndexs->SetData(0);
		BindShaderStorage(4, pCubeIndexs->Handle());
		BindShaderStorage(5, pTaskNums->Handle());
		BindUniform(m_uTaskNum, (UInt)GetTaskThreadNum());
	}
}

}
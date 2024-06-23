#include "GLTFSceneUpdater.h"
#include "GLTFStruct.h"
namespace KI
{

GLTFSceneMatrixUpdaterOnGpu::GLTFSceneMatrixUpdaterOnGpu()
	: m_pBuffer(nullptr)
	, m_pShader(nullptr)
	, m_maxLevel(0)
{
}

GLTFSceneMatrixUpdaterOnGpu::~GLTFSceneMatrixUpdaterOnGpu()
{
	RELEASE_INSTANCE(m_pBuffer);
}
void GLTFSceneMatrixUpdaterOnGpu::CalcMaxLevel(const Vector<GLTFNode>& nodeTree)
{
	CalcMaxLevelRecursive(nodeTree, 0, 0);
	m_maxLevel += 1;
}

void GLTFSceneMatrixUpdaterOnGpu::CalcMaxLevelRecursive(const Vector<GLTFNode>& nodeTree, int index, int level)
{
	m_maxLevel = std::max(level, m_maxLevel);
	for (int i = 0; i < nodeTree[index].GetChild().size(); i++) {
		CalcMaxLevelRecursive(nodeTree, nodeTree[index].GetChild()[i], level + 1);
	}
}
void GLTFSceneMatrixUpdaterOnGpu::CreateRecursive(const Vector<GLTFNode>& nodeTree, int index, int level, const std::vector<int>& treeIndex)
{
	auto childIndex = treeIndex;
	childIndex[level] = index;
	m_map[index] = childIndex;
	const auto& childs = nodeTree[index].GetChild();
	for (size_t i = 0; i < childs.size(); i++) {
		CreateRecursive(nodeTree, childs[i], level + 1, childIndex);
	}
}

void GLTFSceneMatrixUpdaterOnGpu::Printf()
{
	for (int i = 0; i < m_map.size(); i++) {
		std::string line;
		for (int j = 0; j < m_map[i].size(); j++) {
			line += IntToString(m_map[i][j]) + ",";
		}

		line += "\r\n";
		printf(line.c_str());
	}
}

void GLTFSceneMatrixUpdaterOnGpu::Initialize(const Vector<GLTFNode>& nodeTree)
{
	m_map.resize(nodeTree.size());
	CalcMaxLevel(nodeTree);
	for (size_t i = 0; i < nodeTree.size(); i++) {
		if (m_map[i].size() != m_maxLevel) {
			std::vector<int> indecies(m_maxLevel, -1);
			CreateRecursive(nodeTree, i, 0, indecies);
		}
	}
	Printf();

	assert(m_map.size() != 0);
	m_pBuffer = new GLBuffer();
	m_pBuffer->Create(m_map.size() * m_maxLevel, sizeof(int));
	for (size_t i = 0; i < m_map.size(); i++) {
		m_pBuffer->BufferSubData(i * m_maxLevel, m_map[i]);
	}

	m_pShader = new Shader();
	m_pShader->Build();
}

void GLTFSceneMatrixUpdaterOnGpu::Execute(GLBuffer* pNodeBuffer)
{
	m_pShader->Execute(m_pBuffer, pNodeBuffer, m_maxLevel);
}

void GLTFSceneMatrixUpdaterOnGpu::Shader::Execute(GLBuffer* pMapBuffer, GLBuffer* pNodeBuffer, int maxLevel)
{
	Use();
	glUniform1i(m_uniformMaxLevel, maxLevel);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pNodeBuffer->Handle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pMapBuffer->Handle());
	glDispatchCompute(pNodeBuffer->Num(), 1, 1);
	UnUse();
}

ShaderPath GLTFSceneMatrixUpdaterOnGpu::Shader::GetShaderPath()
{
	ShaderPath path;
	path.header.push_back("version.h");
	path.header.push_back("gltf\\gltf.h");
	path.shader[SHADER_PROGRAM_COMPUTE] = "gltf\\gltfMatrixUpdate.comp";
	return path;
}

void GLTFSceneMatrixUpdaterOnGpu::Shader::GetUniformLocation()
{
	m_uniformMaxLevel = glGetUniformLocation(m_programId, "u_maxLevel");
}


void GLTFChannelUpdaterOnGpu::Execute(GLBuffer* pNodeBuffer, GLBuffer* pChannelBuffer, GLBuffer* pSamplerBuffer, float timer)
{
	if (m_pShader == nullptr) {
		m_pShader = new Shader();
		m_pShader->Build();
	}

	m_pShader->Execute(pNodeBuffer, pChannelBuffer, pSamplerBuffer, timer);
}
ShaderPath GLTFChannelUpdaterOnGpu::Shader::GetShaderPath()
{
	ShaderPath path;
	path.header.push_back("version.h");
	path.header.push_back("quart.h");
	path.header.push_back("gltf\\gltf.h");
	path.shader[SHADER_PROGRAM_COMPUTE] = "gltf\\gltfChannelUpdate.comp";
	return path;
}

void GLTFChannelUpdaterOnGpu::Shader::GetUniformLocation()
{
	m_uniformTimer = glGetUniformLocation(m_programId, "u_Timer");
}


void GLTFChannelUpdaterOnGpu::Shader::Execute(GLBuffer* pNodeBuffer, GLBuffer* pChannelBuffer, GLBuffer* pSamplerBuffer, float timer)
{
	Use();
	glUniform1f(m_uniformTimer, timer);
	OUTPUT_GLERROR;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pNodeBuffer->Handle());
	OUTPUT_GLERROR;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pChannelBuffer->Handle());
	OUTPUT_GLERROR;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, pSamplerBuffer->Handle());
	OUTPUT_GLERROR;
	glDispatchCompute(pChannelBuffer->Num(), 1, 1);
	OUTPUT_GLERROR;

	UnUse();
}

void GLTFSkinUpdaterOnGpu::Execute(GLBuffer* pNodeBuffer, GLBuffer* pSkinBuffer, int jointNum)
{
	if (!m_pShader) {
		m_pShader = new Shader();
		m_pShader->Build();
	}

	m_pShader->Execute(pNodeBuffer, pSkinBuffer, jointNum);
}

ShaderPath GLTFSkinUpdaterOnGpu::Shader::GetShaderPath()
{
	ShaderPath path;
	path.header.push_back("version.h");
	path.header.push_back("quart.h");
	path.header.push_back("gltf\\gltf.h");
	path.shader[SHADER_PROGRAM_COMPUTE] = "gltf\\gltfSkinUpdate.comp";
	return path;
}

void GLTFSkinUpdaterOnGpu::Shader::Execute(GLBuffer* pNodeBuffer, GLBuffer* pSkinBuffer, int jointNum)
{
	Use();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pNodeBuffer->Handle());
	OUTPUT_GLERROR;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pSkinBuffer->Handle());
	OUTPUT_GLERROR;
	glDispatchCompute(jointNum, 1, 1);
	OUTPUT_GLERROR;
	UnUse();
}
};



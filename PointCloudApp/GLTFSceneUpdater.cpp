#include "GLTFSceneUpdater.h"
#include "GLTFStruct.h"
namespace KI
{

GLTFSceneMatrixUpdaterOnGpu::GLTFSceneMatrixUpdaterOnGpu()
	:m_pBuffer(nullptr)
	,m_maxLevel(0)
{
}

GLTFSceneMatrixUpdaterOnGpu::~GLTFSceneMatrixUpdaterOnGpu()
{
	if (m_pBuffer) {
		delete m_pBuffer;
		m_pBuffer = nullptr;
	}
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

Vector<String> GLTFSceneMatrixUpdaterOnGpu::Shader::GetHeaderPath()
{
	Vector<String> path;
	path.push_back("E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\Shader\\gltf.h");
	return path;
}

String GLTFSceneMatrixUpdaterOnGpu::Shader::GetComputePath()
{
	return "E:\\MyProgram\\KIProject\\PointCloudApp\\PointCloudApp\\ComputeShader\\gltfMatrixUpdate.comp";
}

void GLTFSceneMatrixUpdaterOnGpu::Shader::GetUniformLocation()
{
	m_uniformMaxLevel = glGetUniformLocation(m_programId, "u_maxLevel");
}


GLTFSceneMatrixUpdaterOnCpu::~GLTFSceneMatrixUpdaterOnCpu()
{
	if (m_pNodeBuffer) {
		delete m_pNodeBuffer;
		m_pNodeBuffer = nullptr;
	}
}
void GLTFSceneMatrixUpdaterOnCpu::Initialize(const Vector<GLTFNode>& nodes, Vector<GLTFSkin>& skins)
{

};
};



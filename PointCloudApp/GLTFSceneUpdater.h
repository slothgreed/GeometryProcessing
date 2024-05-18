#ifndef KI_GLTF_SCENE_UPDATER_H
#define KI_GLTF_SCENE_UPDATER_H
#include "GLTFStruct.h"
#include "IShader.h"
namespace KI
{
// ノードのインデックスを基に2次元行列に格納する。
// GPUでノードのインデックスを計算できるようにするためのマップ
// 行がノードインデックス・列がそのインデックスをたどったインデックス
class GLTFSceneMatrixUpdaterOnGpu
{
public:
	GLTFSceneMatrixUpdaterOnGpu();
	~GLTFSceneMatrixUpdaterOnGpu();

	void Initialize(const Vector<GLTFNode>& node);
	void Execute(GLBuffer* pNodeBuffer);

	int GetMaxLevel() const { return m_maxLevel; }
	const Vector<Vector<int>>& GetMap() const { return m_map; }
private:
	void CalcMaxLevel(const Vector<GLTFNode>& nodeTree);
	void CalcMaxLevelRecursive(const Vector<GLTFNode>& nodeTree, int index, int level);
	void CreateRecursive(const Vector<GLTFNode>& nodeTree, int index, int level, const std::vector<int>& treeIndex);
	void Printf();
	GLBuffer* m_pBuffer;
	Vector<Vector<int>> m_map;
	int m_maxLevel;

	class Shader : public IComputeShader
	{
	public:
		Shader()
			:m_uniformMaxLevel(0)
		{
		}
		virtual ShaderPath GetShaderPath();
		virtual void GetUniformLocation();
		void Execute(GLBuffer* pMapBuffer, GLBuffer* pNodeBuffer, int maxLevel);
	private:

		GLuint m_uniformMaxLevel;
	};

	Shader* m_pShader;

};

class GLTFChannelUpdaterOnGpu
{
public:
	GLTFChannelUpdaterOnGpu() : m_pShader(nullptr) {};
	~GLTFChannelUpdaterOnGpu() {};

	void Execute(GLBuffer* pNodeBuffer, GLBuffer* pChannelBuffer, GLBuffer* pSamplerBuffer, float timer);
private:

	class Shader : public IComputeShader
	{
	public:
		Shader() :
			m_uniformTimer(0) {};
		~Shader() {};

		virtual ShaderPath GetShaderPath();
		virtual void GetUniformLocation();
		void Execute(GLBuffer* pNodeBuffer, GLBuffer* pChannelBuffer, GLBuffer* pSamplerBuffer, float timer);
	private:
		GLuint m_uniformTimer;
	};

	Shader* m_pShader;
};

class GLTFSkinUpdaterOnGpu
{
public:
	GLTFSkinUpdaterOnGpu()
		: m_pShader(nullptr)
	{
	}
	~GLTFSkinUpdaterOnGpu() {};

	void Execute(GLBuffer* pNodeBuffer, GLBuffer* pSkinBuffer, int jointNum);
	class Shader : public IComputeShader
	{
	public:
		Shader() {};
		virtual ~Shader() {};

		virtual ShaderPath GetShaderPath();
		virtual void GetUniformLocation() {};

		void Execute(GLBuffer* pNodeBuffer, GLBuffer* pSkinBuffer, int jointNum);
	private:

	};

private:
	Shader* m_pShader;
};


}

#endif KI_GLTF_SCENE_UPDATER_H
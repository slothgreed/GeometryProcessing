#ifndef ALGORITHM_SHADER_H
#define ALGORITHM_SHADER_H
#include "IShader.h"
#include "Texture.h"

namespace KI
{
class MarchingCubeShader : public IMeshShader
{
public:
	virtual int GetTaskThreadNum() const { return 32; }
	MarchingCubeShader(int voxelSize, bool withTask)
		: m_voxelSize(voxelSize)
		, m_withTask(withTask)
		, m_uTaskNum(0)	
		, m_dataType(DATA_TYPE::DATA_USHORT){};
	MarchingCubeShader(int voxelSize, bool withTask, DATA_TYPE dataType)
		: m_voxelSize(voxelSize)
		, m_withTask(withTask)
		, m_uTaskNum(0)
		, m_dataType(dataType)
	{
	};
	~MarchingCubeShader() {};

	virtual ShaderPath GetShaderPath();
	virtual void FetchUniformLocation();

	virtual void Build();
	bool WithTask() const { return m_withTask; }
	void SetCamera(const GLBuffer* pBuffer);
	void SetVoxel(const GLBuffer* pBuffer);
	void SetVoxelData(const GLBuffer* pBuffer);
	void SetModel(const Matrix4x4& value);
	void SetThreshold(float value);
	void PreDraw();
private:
	DATA_TYPE m_dataType;
	Unique<GLBuffer> pTriTable = nullptr;
	Unique<GLBuffer> pCubeIndexs = nullptr;
	Unique<GLBuffer> pTaskNums = nullptr;
	int m_voxelSize;
	bool m_withTask;
	GLuint m_uModel;
	GLuint m_uThreshold;
	GLuint m_uTaskNum;
};

}
#endif ALGORITHM_SHADER_H
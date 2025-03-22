#ifndef VOXELIZER_H
#define VOXELIZER_H
#include "IAlgorithm.h"
#include "IShader.h"
#include "BDB.h"
#include "Utility.h"

namespace KI
{
class HalfEdgeNode;
class Voxelizer : public IAlgorithm
{
public:
	Voxelizer(HalfEdgeNode* pNode);
	virtual ~Voxelizer();

	virtual ALGORITHM_TYPE GetType() override { return ALGORITHM_VOXELIZER; }
	void Execute(GLBuffer* pointBuffer, GLBuffer* indexBuffer);

	void Draw(GLBuffer* pointBuffer, GLBuffer* indexBuffer, int camera);
	virtual void ShowUI(UIContext& ui);

private:
	void ExecuteCPU(const BDB& bdb, int triIdx, const std::vector<Vector3>& position, const std::vector<unsigned int>& indexs, std::vector<unsigned int>& results);

	struct MeshShader : IMeshShader
	{
		enum UNIFORM
		{
			MIN,
			MODEL,
			PITCH,
			RESOLUTE,
			ARRAYSIZE,
			NUM
		};
		virtual void FetchUniformLocation();
		virtual ShaderPath GetShaderPath();
		void Draw(int camera, const Matrix4x4& matrix, const BDB& bdb, int resolution, GLBuffer* pBuffer);

		GLuint m_uniform[UNIFORM::NUM];
	};

	struct ComputeShader : IComputeShader
	{
		ComputeShader();
		virtual ~ComputeShader();
		friend struct Voxelizer::MeshShader;
		enum UNIFORM
		{
			MIN,
			PITCH,
			RESOLUTE,
			TRIANGLENUM,
			NUM
		};


		virtual void FetchUniformLocation();
		virtual ShaderPath GetShaderPath();

		GLBuffer* m_pVoxelBuffer;
		std::vector<unsigned int> Execute(const BDB& bdb, int resolution, GLBuffer* pointBuffer, GLBuffer* indexBuffer, int triangleNum);
		GLuint m_uniform[UNIFORM::NUM];
	};

	GLBuffer* m_pVec4PointBuffer;
	GLBuffer* m_pCpuVoxel;
	ComputeShader m_gpuVoxelizer;
	MeshShader m_drawer;
	HalfEdgeNode* m_pNode;
	int m_resolution;
};

}
#endif VOXELIZER_H
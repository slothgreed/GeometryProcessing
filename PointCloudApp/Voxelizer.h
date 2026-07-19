#ifndef VOXELIZER_H
#define VOXELIZER_H
#include "IAlgorithm.h"
#include "IShader.h"
#include "BDB.h"
#include "Utility.h"
#include "Voxel.h"
namespace KI
{
class HalfEdgeNode;
class Voxelizer : public IAlgorithm
{
private:
	struct CompactVoxel
	{
		CompactVoxel() {};
		CompactVoxel(int resolution, std::vector<unsigned int>&& data)
			: m_resolution(resolution)
			, m_data(std::move(data))
		{
		}
		static int Calc1DArraySize(int resolution) { return UIntBool::Size(resolution) * resolution * resolution; }
		Vector3i GetIndex(uint location, int bitIndex) const;
		bool IsBoundary(const Vector3i& index) const;
		void SetBoundary(const Vector3i& index);
		const std::vector<unsigned int>& GetData() const { return m_data; }
	private:
		std::vector<unsigned int> m_data;
		int m_resolution = 0;
	};

public:

	static const char* const* GetLabelString();
	typedef std::vector<std::vector<std::vector<char>>> VoxelLabel;

	Voxelizer(HalfEdgeNode* pNode);
	virtual ~Voxelizer();

	virtual ALGORITHM_TYPE GetType() override { return ALGORITHM_VOXELIZER; }
	void Execute(int resolute);
	CompactVoxel ExecuteCPU(int resolute);
	void Draw(GLBuffer* pointBuffer, GLBuffer* indexBuffer, int camera);
	virtual void ShowUI(RenderNode* pNode, UIContext& ui);

	void CreateLabelPoint(Vector<Vector3>& position, Vector<Vector3>& color, VOXEL_LABEL type);
	
	int GetResolution() const { return m_resolution; }
	BDB GetCellBDB(const ivec3& index) const;
	VoxelLabel CreateLabel() const;
private:
	float GetPitch() const;
	Vector3 GetCenter(const ivec3& index) const;
	bool InVoxel(const ivec3& index) const;
	void ExecuteCPU(const BDB& bdb, int triIdx, const std::vector<Vector3>& position, const std::vector<unsigned int>& indexs, std::vector<unsigned int>& results);

	struct UI
	{
		UI() 
			:visibleInOut(false)
			,label(1) {}
		bool visibleInOut;
		int label;
	};




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
	UI m_ui;
	VoxelLabel m_labels;
	CompactVoxel m_compactVoxel;
};

}
#endif VOXELIZER_H
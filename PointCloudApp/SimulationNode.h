#ifndef SIMULATION_NODE
#define SIMULATION_NODE
#include "RenderNode.h"
namespace KI
{
class SimulationNode : public RenderNode
{
public:
	SimulationNode();
	~SimulationNode() {};

	virtual void DrawNode(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

private:
	Vector<int> GetNeighbors(int i);
	void Initialize();
	void Simulation();


	struct Particle
	{
		Vector4 position; // x,y,z, cellIndex;
		Vector4 velocity;

		Vector3 GetPosition() const { return Vector3(position.x, position.y, position.z); }
	};

	struct Grid
	{
		Grid() : cell() {}
		Vector<Vector4u> cell; // offset, num, density;
		std::vector<int> particleIndex;

		int Offset(int index) const { return cell[index].x; }
		int Num(int index) const { return cell[index].y; }
		void SetOffset(int cellId, int offset) { cell[cellId].x = offset; }
		void AddNum(int cellId) { cell[cellId].y += 1; }
		void SetParticleIndex(int index, int particleIndex_) { particleIndex[index] = particleIndex_; }
		int ParticleIndex(int particleIndex_) { return particleIndex[particleIndex_]; }
		void Initialize()
		{
			std::fill(cell.begin(), cell.end(), Vector4u(0));
			std::fill(particleIndex.begin(), particleIndex.end(), 0);
		}
		void Allocate(int cellSize, int particleSize)
		{
			cell.resize(cellSize); std::fill(cell.begin(), cell.end(), Vector4u(0));
			particleIndex.resize(particleSize); std::fill(particleIndex.begin(), particleIndex.end(), 0);
		}
		int Size() const { return cell.size(); }
		void AddDensity(int index, float value)
		{
			unsigned int uValue = 0;
			std::memcpy(&uValue, &value, sizeof(float));
			cell[index].z += uValue;
		}
	};

	struct SettingGpu
	{
		Vector4i cellNum;
		Vector4 spaceMin;
		Vector4 spaceMax;
		int particleNum;
		int gridNum;
		int pad1;
		int pad2;
	};


	class PositionUpdater : public IComputeShader
	{
	public:
		PositionUpdater() {};
		virtual ~PositionUpdater() {};
		virtual ShaderPath GetShaderPath();
	private:

	};

	class VelocityUpdater : public IComputeShader
	{
	public:
		VelocityUpdater() {};
		virtual ~VelocityUpdater() {};
		virtual ShaderPath GetShaderPath();
	private:

	};

	class DensityUpdater : public IComputeShader
	{
	public:
		DensityUpdater() {};
		virtual ~DensityUpdater() {};
		virtual ShaderPath GetShaderPath();
	private:

	};

	class CellIDUpdater : public IComputeShader
	{
	public:
		CellIDUpdater(int phase)
			: m_phase(phase) {};
		virtual ~CellIDUpdater() {};
		virtual ShaderPath GetShaderPath();
	private:
		int m_phase;
	};

	class CellIDUpdaterPrefixSum : public IComputeShader
	{
	public:
		CellIDUpdaterPrefixSum(int phase)
			:m_phase(phase) {};
		virtual ~CellIDUpdaterPrefixSum() {};
		virtual ShaderPath GetShaderPath();
		virtual Vector3i GetLocalThreadNum() const;

		static int GetBlockNum() { return 64; }
	private:
		int m_phase;
	};

	struct Gpu
	{
		Gpu()
			: pPositionUpdater(nullptr)
			, pDensityUpdater(nullptr)
			, pVelocityUpdater(nullptr)
			, pPosition(nullptr)
			, pParticle(nullptr)
			, pParticleIndex(nullptr)
			, pGridOffset(nullptr)
			, pPrefixSumChunk(nullptr)
			, pGridCell(nullptr)
			, pSetting(nullptr)
			, pVelocity(nullptr)
		{
		}
		Unique<DensityUpdater> pDensityUpdater;
		Unique<VelocityUpdater> pVelocityUpdater;
		Unique<PositionUpdater> pPositionUpdater;
		std::array<Unique<CellIDUpdater>, 2> pCellIDUpdater;
		std::array<Unique<CellIDUpdaterPrefixSum>, 3> pCellIDPrefixSum;
		Unique<GLBuffer> pPrefixSumChunk;
		Unique<GLBuffer> pGridOffset;
		Unique<GLBuffer> pParticle;
		Unique<GLBuffer> pGridCell;
		Unique<GLBuffer> pParticleIndex;
		Unique<GLBuffer> pSetting;
		Unique<GLBuffer> pPosition;
		Unique<GLBuffer> pVelocity;
	};

	enum PHASE
	{
		POSITION,
		DENSITY,
		CELLID,
		VELOCITY
	};

	float Poly6FromR2(float r, float h);
	Vector3 GetCellCenter(const Vector3i& pos);
	void UpdatePosition();
	void UpdateCellID();
	void UpdateDensity();
	void UpdateVelocity();
	Vector3i GetCellID(const Vector3& cellID);
	int ToIndex1D(const Vector3i& cellID);
	Vector3i m_cellNum;
	Grid m_grid;
	BDB m_space;
	Gpu m_gpu;
	Vector<Particle> m_particles;
	BitSet<4> m_useGpu;
	bool m_debug;
	int m_simulationCount;
	void TestPosition();
	
};

}

#endif SIMULATION_NODE

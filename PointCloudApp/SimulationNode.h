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
	struct Gpu
	{
		Unique<GLBuffer> pPosition;
		Unique<GLBuffer> pVelocity;
	};

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
		std::vector<int> m_particleIndex;

		int Offset(int index) const { return cell[index].x; }
		int Num(int index) const { return cell[index].y; }
		void SetOffset(int cellId, int offset) { cell[cellId].x = offset; }
		void AddNum(int cellId) { cell[cellId].y += 1; }
		void SetParticleIndex(int index, int particleIndex) { m_particleIndex[index] = particleIndex; }
		int ParticleIndex(int particleIndex) { return m_particleIndex[particleIndex]; }
		void Allocate(int cellSize, int particleSize)
		{
			cell.resize(cellSize); std::fill(cell.begin(), cell.end(), Vector4u(0));
			m_particleIndex.resize(particleSize); std::fill(m_particleIndex.begin(), m_particleIndex.end(), 0);
		}
		int Size() const { return cell.size(); }
		void AddDensity(int index, float value)
		{
			unsigned int uValue = 0;
			std::memcpy(&uValue, &value, sizeof(float));
			cell[index].z += uValue;
		}
	};


	class PositionUpdater : public IComputeShader
	{
	public:
		PositionUpdater();
		~PositionUpdater();

		void BindParticle(const GLBuffer* pBuffer);
	private:

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
	
};

}

#endif SIMULATION_NODE

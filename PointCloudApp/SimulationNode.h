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
		Vector4 position;
		Vector4 velocity;
	};

	Gpu m_gpu;
	Vector<Particle> m_particles;
	
};

}

#endif SIMULATION_NODE

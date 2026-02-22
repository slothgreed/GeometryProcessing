#ifndef SHAPE_MATCHING_H
#define SHAPE_MATCHING_H
#include "KMeansAlgorithm.h"
#include "HalfEdgeNode.h"
namespace KI
{
class ShapeMatching
{
public:
	ShapeMatching(HalfEdgeNode* pNode);
	~ShapeMatching();

	void Initialize();
	void Update(float dt);

	const GLBuffer* GetPosition() const { return m_pPositionBuffer.get(); };
private:
	void UpdateCpu(float dt);
	void UpdateGpu();

	struct Particles
	{
		Particles() {}
		void Allocate(int size)
		{
			m_initPoint.resize(size, Vector3(0));
			m_beforePosition.resize(size, Vector3(0));
			m_position.resize(size, Vector3(0));
			m_normal.resize(size, Vector3(0));
			m_velocity.resize(size,Vector3(0));
			m_weight.resize(size,0.0);
		}
		size_t Size() const { return m_position.size(); }
		Vector<Vector3> m_initPoint;
		Vector<Vector3> m_beforePosition;
		Vector<Vector3> m_position;
		Vector<Vector3> m_normal;
		Vector<Vector3> m_velocity;
		Vector<float> m_weight;
	};

	struct VertexWeight
	{
		VertexWeight(int c, float v) { index = c; weight = v; }
		int index;
		float weight;
	};
	Vector3 ComputeCenter(const Vector<VertexWeight>& cluster, bool rest);
	Matrix4x4 ComputeApq(const Vector<VertexWeight>& c, const Vector3& cp, const Vector3& cq);


	HalfEdgeNode* m_pHalfEdge;
	int m_clusterNum;
	Vector<Vector<VertexWeight>> m_cluster;
	Particles m_particle;
	Unique<GLBuffer> m_pPositionBuffer;
	Unique<GLBuffer> m_pNormalBuffer;

};

}

#endif SHAPE_MATCHING_H

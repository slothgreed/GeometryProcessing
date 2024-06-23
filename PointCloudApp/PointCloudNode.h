#ifndef POINT_CLOUD_NODE_H
#define POINT_CLOUD_NODE_H
#include "RenderNode.h"
#include "SimpleShader.h"
#include "IAlgorithm.h"
namespace KI
{

class PointCloud;
class PointCloudNode : public RenderNode
{
public:
	PointCloudNode(const String& name, Shared<PointCloud>& pPrimitive);
	~PointCloudNode();

	const Shared<PointCloud>& GetData() const;
	void UpdateData();
	void DrawNode(const DrawContext& context);
	virtual void ShowUI();
	const Vector<Vector3>& GetNormal();
	const Vector<int>& GetNeighbor(int index);
private:
	void ComputeNormal();
	void ComputeNeighbor(float radius);
	void ShowNormal();
	void ComputeTangent();
	void BuildGLBuffer();
	void UpdateRenderData();
	Vector<Vector3> m_normal;
	Vector<Vector3> m_tangentX;
	Vector<Vector3> m_tangentY;
	std::unordered_map<ALGORITHM_TYPE, IAlgorithm*> m_algorithm;
	Shared<VertexColorShader> m_pShader;
	Shared<PointCloud> m_pPointCloud;
	Vector<Vector<int>> m_neighbor;
	Unique<GLBuffer> m_pPositionBuffer;
	Unique<GLBuffer> m_pColorBuffer;
};

}

#endif POINT_CLOUD_NODE_H
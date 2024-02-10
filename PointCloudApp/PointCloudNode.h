#ifndef POINT_CLOUD_NODE_H
#define POINT_CLOUD_NODE_H
#include "VertexColorShader.h"
#include "RenderNode.h"
#include "IAlgorithm.h"

class PointCloud;
class PointCloudNode : public RenderNode
{
public:
	PointCloudNode(const String& name, Shared<PointCloud>& pPrimitive);
	~PointCloudNode();

	const Shared<PointCloud>& GetData() const;
	void UpdateData();
	void DrawData(const mat4x4& proj, const mat4x4& view);
	virtual void ShowUI();
	const Vector<vec3>& GetNormal();
	const Vector<int>& GetNeighbor(int index);
private:
	void ComputeNormal();
	void ComputeNeighbor(float radius);
	void ShowNormal();
	void ComputeTangent();
	void BuildGLBuffer();
	void UpdateRenderData();
	Vector<vec3> m_normal;
	Vector<vec3> m_tangentX;
	Vector<vec3> m_tangentY;
	std::unordered_map<ALGORITHM_TYPE, IAlgorithm*> m_algorithm;
	Shared<VertexColorShader> m_pShader;
	Shared<PointCloud> m_pPointCloud;
	Vector<Vector<int>> m_neighbor;
	std::unique_ptr<GLBuffer> m_pPositionBuffer;
	std::unique_ptr<GLBuffer> m_pColorBuffer;
};



#endif POINT_CLOUD_NODE_H
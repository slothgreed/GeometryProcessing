#ifndef POINT_CLOUD_NODE_H
#define POINT_CLOUD_NODE_H
#include "VertexColorShader.h"
#include "RenderNode.h"
#include "IAlgorithm.h"

class PointCloud;
class PointCloudNode : public RenderNode
{
public:
	PointCloudNode(const string& name, std::shared_ptr<PointCloud>& pPrimitive);
	~PointCloudNode();

	const std::shared_ptr<PointCloud>& GetData() const;
	void UpdateData();
	void DrawData(const mat4x4& proj, const mat4x4& view);
	virtual void ShowUI();
	const std::vector<vec3>& GetNormal();
	const std::vector<int>& GetNeighbor(int index);
private:
	void ComputeNormal();
	void ComputeNeighbor(float radius);
	void ShowNormal();
	void ComputeTangent();
	void BuildGLBuffer();
	void UpdateRenderData();
	std::vector<vec3> m_normal;
	std::vector<vec3> m_tangentX;
	std::vector<vec3> m_tangentY;
	std::unordered_map<ALGORITHM_TYPE, IAlgorithm*> m_algorithm;
	std::shared_ptr<VertexColorShader> m_pShader;
	std::shared_ptr<PointCloud> m_pPointCloud;
	std::vector<std::vector<int>> m_neighbor;
	std::unique_ptr<GLBuffer> m_pPositionBuffer;
	std::unique_ptr<GLBuffer> m_pColorBuffer;
};



#endif POINT_CLOUD_NODE_H
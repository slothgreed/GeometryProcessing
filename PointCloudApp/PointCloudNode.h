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
private:
	void BuildGLBuffer();
	void UpdateRenderData();
	std::unordered_map<ALGORITHM_TYPE, IAlgorithm*> m_algorithm;
	std::shared_ptr<VertexColorShader> m_pShader;
	std::shared_ptr<PointCloud> m_pPointCloud;
	std::unique_ptr<GLBuffer> m_pPositionBuffer;
	std::unique_ptr<GLBuffer> m_pColorBuffer;
};



#endif POINT_CLOUD_NODE_H
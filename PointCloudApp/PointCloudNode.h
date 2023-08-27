#ifndef POINT_CLOUD_NODE_H
#define POINT_CLOUD_NODE_H
#include "SimpleShader.h"
class PointCloud;
class PointCloudNode
{
public:
	PointCloudNode();
	~PointCloudNode();

	void Set(std::unique_ptr<PointCloud>&& pPrimitive);
	const std::unique_ptr<PointCloud>& GetData() const;
	void UpdateData();
	void Draw(const mat4x4& proj, const mat4x4& view);

private:
	void BuildGLBuffer();
	void UpdateRenderData();
	std::unique_ptr<SimpleShader> m_pShader;
	std::unique_ptr<PointCloud> m_pPointCloud;
	std::unique_ptr<GLBuffer> m_pPositionBuffer;
	std::unique_ptr<GLBuffer> m_pColorBuffer;
};



#endif POINT_CLOUD_NODE_H
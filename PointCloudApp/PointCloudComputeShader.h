#ifndef POINT_CLOUD_COMPUTE_SHADER_H
#define POINT_CLOUD_COMPUTE_SHADER_H
#include "IShader.h"
class PointCloud;
class Texture2D;
class PointCloudComputeShader : public IComputeShader
{
public:
	PointCloudComputeShader(const Shared<PointCloud>& pPointCloud);
	~PointCloudComputeShader();

	virtual String GetComputePath();
	virtual void GetUniformLocation();
	virtual void Execute(const mat4x4& proj, const mat4x4& view, const Shared<Texture2D>& pTexture, const Shared<Texture2D>& pDepthBuffer);
private:
	void Initialize();
	Shared<PointCloud> m_pPointCloud;
	std::unique_ptr<GLBuffer> m_pPointBuffer;
	std::unique_ptr<GLBuffer> m_pColorBuffer;
	GLuint m_uniformVP;
};

#endif POINT_CLOUD_COMPUTE_SHADER_H
#ifndef COMPUTE_POINT_CLOUD_APP_H
#define COMPUTE_POINT_CLOUD_APP_H
#include "GLFWApp.h"
#include "CameraController.h"

namespace KI
{
class PointCloud;
class Texture;
class Texture2D;
class CreateTextureComputeShader : public IComputeShader
{
public:
	CreateTextureComputeShader() {};
	~CreateTextureComputeShader() {};

	virtual ShaderPath GetShaderPath() override;
	void Execute(const Shared<Texture>& pTexture);
	virtual void FetchUniformLocation() override;
private:

};

class PointCloudComputeShader : public IComputeShader
{
public:
	PointCloudComputeShader(const Shared<PointCloud>& pPointCloud);
	~PointCloudComputeShader();
	virtual Vector3i GetLocalThreadNum() const { return Vector3i(256, 1, 1); }
	virtual ShaderPath GetShaderPath();
	virtual void FetchUniformLocation();
	virtual void Execute(const Matrix4x4& proj, const Matrix4x4& view, const Shared<Texture2D>& pTexture, const Shared<Texture2D>& pDepthBuffer);
private:
	void Initialize();
	Shared<PointCloud> m_pPointCloud;
	Unique<GLBuffer> m_pPointBuffer;
	GLuint m_uniformVP;
};

class ComputePointCloudApp : public GLFWApp
{
public:
	ComputePointCloudApp() {};
	~ComputePointCloudApp() {};

	virtual void Execute();
	void ProcessMouseEvent(const MouseInput& input);
	void ResizeEvent(int width, int height);

private:
	Shared<Texture2D> m_pColorTexture;
};

}

#endif COMPUTE_POINT_CLOUD_APP_H
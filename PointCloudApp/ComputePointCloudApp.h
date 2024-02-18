#ifndef COMPUTE_POINT_CLOUD_APP_H
#define COMPUTE_POINT_CLOUD_APP_H
#include "GLFWApp.h"
#include "CameraController.h"

namespace KI
{

class Texture2D;
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
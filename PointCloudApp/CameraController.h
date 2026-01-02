#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "Camera.h"
#include "MouseController.h"
namespace KI
{
class CameraController : public MouseController
{
public:
	CameraController(const Shared<Camera>& pCamera);
	~CameraController() {};
	virtual bool Move(const EditContext& mouse);
	virtual bool Wheel(const EditContext& mouse);
	void SetAspect(float width, float height);
	void RotateAnimation(float time, const BDB& bdb);
	void FitToBDB(const BDB& bdb);
	
private:
	void Zoom(float ratio);
	void Rotate(const vec2& move);
	void Translate(const vec2& move);
	Shared<Camera> m_pCamera;
	float m_ZoomRatio;
	float m_oldAspect;

};
}
#endif CAMERA_CONTROLLER_H
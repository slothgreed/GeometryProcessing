#include "CameraController.h"
#include "PerspectiveCamera.h"

bool CameraController::Move(const Mouse& mouse)
{
	if (mouse.Press(MY_MOUSE_BUTTON::MOUSE_BUTTON_RIGHT))
	{
		vec2 move = mouse.Delta();
		move.x *= 0.3f;
		move.y *= -0.3f;
		Rotate(move);
	}
	else if(mouse.Press(MY_MOUSE_BUTTON::MOUSE_BUTTON_MIDDLE))
	{
		vec2 move = mouse.Delta();
		move.x *= -0.3f;
		move.y *= 0.3f;
		Translate(move);
	}

	return true;
}

bool CameraController::Wheel(const Mouse&  mouse)
{
	if (mouse.Wheel() > 0)
	{
		Zoom(1 + m_ZoomRatio);
	}
	else
	{
		Zoom(1 - m_ZoomRatio);
	}

	return true;
}

void CameraController::Zoom(float ratio)
{
	auto pCamera = (PerspectiveCamera*)m_pCamera.get();
	vec3 eyeDirect = pCamera->Direction();
	float len = pCamera->LookAtDistance() * ratio;

	vec3 newEye = eyeDirect * len + m_pCamera->Center();

	m_pCamera->LookAt(newEye, pCamera->Center(), pCamera->Up());
}

void CameraController::Rotate(const vec2& move)
{
	auto pCamera = (PerspectiveCamera*)m_pCamera.get();
	pCamera->MoveWithSpherical(move);
}

void CameraController::Translate(const vec2& move)
{
	auto pCamera = (PerspectiveCamera*)m_pCamera.get();
	vec3 xDir = pCamera->XDirection() * move.x;
	vec3 yDir = pCamera->YDirection() * move.y;
	vec3 eye = pCamera->Eye() + xDir + yDir;
	vec3 center = pCamera->Center() + xDir + yDir;
	pCamera->LookAt(eye, center, pCamera->Up());
}
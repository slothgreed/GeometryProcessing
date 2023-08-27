#include "CameraController.h"
#include "RenderCamera.h"

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
	vec3 eyeDirect = m_pCamera->Direction();
	float len = m_pCamera->LookAtDistance() * ratio;

	vec3 newEye = eyeDirect * len + m_pCamera->Center();

	m_pCamera->LookAt(newEye, m_pCamera->Center(), m_pCamera->Up());
}

void CameraController::Rotate(const vec2& move)
{
	m_pCamera->MoveWithSpherical(move);
}

void CameraController::Translate(const vec2& move)
{
	vec3 xDir = m_pCamera->XDirection() * move.x;
	vec3 yDir = m_pCamera->YDirection() * move.y;
	vec3 eye = m_pCamera->Eye() + xDir + yDir;
	vec3 center = m_pCamera->Center() + xDir + yDir;
	m_pCamera->LookAt(eye, center, m_pCamera->Up());
}
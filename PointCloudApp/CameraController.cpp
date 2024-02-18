#include "CameraController.h"
#include "Camera.h"
namespace KI
{
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
	Vector3 eyeDirect = m_pCamera->Direction();
	float len = m_pCamera->LookAtDistance() * ratio;

	Vector3 newEye = eyeDirect * len + m_pCamera->Center();

	m_pCamera->SetLookAt(newEye, m_pCamera->Center(), m_pCamera->Up());
}

void CameraController::Rotate(const vec2& move)
{
	m_pCamera->MoveWithSpherical(move);
}

void CameraController::Translate(const vec2& move)
{
	Vector3 xDir = m_pCamera->XDirection() * move.x;
	Vector3 yDir = m_pCamera->YDirection() * move.y;
	Vector3 eye = m_pCamera->Eye() + xDir + yDir;
	Vector3 center = m_pCamera->Center() + xDir + yDir;
	m_pCamera->SetLookAt(eye, center, m_pCamera->Up());
}
}
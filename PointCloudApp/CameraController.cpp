#include "CameraController.h"
#include "Camera.h"
namespace KI
{
bool CameraController::Move(const Mouse& mouse)
{
	if (mouse.Press(MY_MOUSE_BUTTON::MOUSE_BUTTON_RIGHT))
	{
		vec2 move = mouse.Delta();
		move.x *= 0.1f;
		move.y *= -0.1f;
		Rotate(move);
	}
	else if(mouse.Press(MY_MOUSE_BUTTON::MOUSE_BUTTON_MIDDLE))
	{
		auto value = 0.01;
		vec2 move = mouse.Delta();
		move.x *= -value;
		move.y *= value;
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

void CameraController::RotateAnimation(float time, const BDB& bdb)
{
	float angle = time * 0.5f; // ‘¬“x’²®
	auto center = bdb.Center();
	auto bdbLength = bdb.MaxLength() * 1.2f;
	float x = center.x + bdbLength * cos(angle);
	float y = center.y;
	float z = center.z + bdbLength * sin(angle);

	m_pCamera->SetEye(Vector3(x, y, z));
}

void CameraController::FitToBDB(const BDB& bdb)
{
	if (!bdb.IsActive()) { return; }
	float lookAtDistance = glm::length(bdb.Max() - bdb.Center()) / (float)sin(m_pCamera->FOV() / 2.0);
	lookAtDistance *= 1.2f / m_pCamera->Aspect();

	Vector3 eyeDirection = m_pCamera->Direction();
	Vector3 newPosition = bdb.Center() + eyeDirection * lookAtDistance;

	m_pCamera->SetLookAt(newPosition, bdb.Center(), m_pCamera->Up());
}

}
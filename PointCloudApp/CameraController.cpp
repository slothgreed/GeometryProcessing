#include "CameraController.h"
#include "Camera.h"
#include "Utility.h"
namespace KI
{
CameraController::CameraController(const Shared<Camera>& pCamera)
	: m_pCamera(pCamera)
	, m_ZoomRatio(0.1f)
	, m_oldAspect(pCamera->Aspect())
{
}
bool CameraController::Move(const EditContext& context)
{
	auto mouse = *context.GetMouse();
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

bool CameraController::Wheel(const EditContext& context)
{
	auto mouse = *context.GetMouse();
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
	if (m_pCamera->IsPerspective()) {
		Vector3 eyeDirect = m_pCamera->Direction();
		float len = m_pCamera->LookAtDistance() * ratio;

		Vector3 newEye = eyeDirect * len + m_pCamera->Center();

		m_pCamera->SetLookAt(newEye, m_pCamera->Center(), m_pCamera->Up());
	} else if (m_pCamera->IsOrtho()) {
		float centerX = (m_pCamera->Left() + m_pCamera->Right()) * 0.5f;
		float centerY = (m_pCamera->Top() + m_pCamera->Bottom()) * 0.5f;
		float halfWidth = (m_pCamera->Right() - m_pCamera->Left()) * 0.5f;
		float halfHeight = (m_pCamera->Top() - m_pCamera->Bottom()) * 0.5f;
		halfWidth *= ratio;
		halfHeight *= ratio;

		auto ortho = m_pCamera->GetOrtho();
		ortho.m_left = centerX - halfWidth;
		ortho.m_right = centerX + halfWidth;
		ortho.m_bottom = centerY - halfHeight;
		ortho.m_top = centerY + halfHeight;


		m_pCamera->SetOrtho(ortho);
	}
}

void CameraController::Rotate(const vec2& move)
{
	m_pCamera->MoveWithSpherical(move);
}

void CameraController::Translate(const vec2& move)
{
	if (m_pCamera->IsPerspective()) {
		Vector3 xDir = m_pCamera->XDirection() * move.x;
		Vector3 yDir = m_pCamera->YDirection() * move.y;
		Vector3 eye = m_pCamera->Eye() + xDir + yDir;
		Vector3 center = m_pCamera->Center() + xDir + yDir;
		m_pCamera->SetLookAt(eye, center, m_pCamera->Up());
	} else {
		//float width = m_pCamera->ViewSize().x / 10;
		//float height = m_pCamera->ViewSize().y / 10;
		Vector3 xDir = m_pCamera->XDirection() * move.x * 100.0f;
		Vector3 yDir = m_pCamera->YDirection() * move.y * 100.0f;
		Vector3 eye = m_pCamera->Eye() + xDir + yDir;
		Vector3 center = m_pCamera->Center() + xDir + yDir;
		m_pCamera->SetLookAt(eye, center, m_pCamera->Up());
	}
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
	if (m_pCamera->IsPerspective()) {
		float lookAtDistance = glm::length(bdb.Max() - bdb.Center()) / (float)sin(m_pCamera->FOV() / 2.0);
		lookAtDistance *= 1.2f / m_pCamera->Aspect();

		Vector3 eyeDirection = m_pCamera->Direction();
		Vector3 newPosition = bdb.Center() + eyeDirection * lookAtDistance;

		m_pCamera->SetLookAt(newPosition, bdb.Center(), m_pCamera->Up());
	} else if (m_pCamera->IsOrtho()) {
		auto size = bdb.Max() - bdb.Min();
		auto center = bdb.Center();
		auto eye = center + glm::vec3(0.0f, 0.0f, 1.0f) * size.z * 2.0f; // ’†S‚Ì­‚µã‚©‚ç
		auto target = center;
		auto up = glm::vec3(0.0f, 1.0f, 0.0f);

		m_pCamera->SetLookAt(eye, target, up);

		float halfWidth = size.x * 0.5f;
		float halfHeight = size.y * 0.5f;
		float nearPlane = 0.0f;
		float farPlane = size.z * 4.0f;
		m_pCamera->SetOrtho(Camera::Ortho(-halfWidth, halfWidth, halfHeight, -halfHeight, nearPlane, farPlane));
	}
}

void CameraController::SetAspect(float width, float height)
{
	float aspect = width / height;
	if (m_pCamera->IsPerspective()) {
		const auto& perspective = m_pCamera->GetPerspective();
		m_pCamera->SetPerspective(perspective.m_fov, aspect, perspective.m_near, perspective.m_far);
	} else {
		auto ortho = m_pCamera->GetOrtho();
		auto center = ortho.Center();
		float halfWidth = ortho.Width() * 0.5f;
		float halfHeight = ortho.Height() * 0.5f;
		if (aspect > 1.0f) {
			halfWidth *= (aspect / m_oldAspect);
		} else {
			halfHeight /= (aspect / m_oldAspect);
		}

		ortho.m_left = center.x - halfWidth;
		ortho.m_right = center.x + halfWidth;
		ortho.m_top = center.y + halfHeight;
		ortho.m_bottom = center.y - halfHeight;
		m_pCamera->SetOrtho(ortho);
	}
	m_oldAspect = aspect;
}

}
#include "Camera.h"
namespace KI
{
Camera::Camera()
{
	m_theta = 0;
	m_phi = 0;
}
void Camera::SetLookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
{
	m_View = glm::lookAt(eye, center, up);
	m_eye = eye;
	m_center = center;
	m_up = up;
	m_direction = glm::normalize(m_eye - m_center);
	m_distance = glm::length(m_eye - m_center);
}

void Camera::SetProject(Matrix4x4 proj)
{
	m_Project = proj;
}

Vector3 Camera::XDirection()
{
	return Vector3(m_View[0].x, m_View[1].x, m_View[2].x);
}

Vector3 Camera::YDirection()
{
	return Vector3(m_View[0].y, m_View[1].y, m_View[2].y);
}
Vector3 Camera::ZDirection()
{
	return Vector3(m_View[0].z, m_View[1].z, m_View[2].z);
}
void Camera::SetPerspective(float fov, float aspect, float _near, float _far)
{
	SetProject(glm::perspective(fov, aspect, _near, _far));
	m_fov = fov;
	m_aspect = aspect;
	m_near = _near;
	m_far = _far;
}

void Camera::SphericalToCartesian(float radius, float x, float y, Vector3& result)
{
	result.x = radius * sin(y) * cos(x);
	result.y = radius * cos(y);
	result.z = radius * sin(y) * sin(x);
}

void Camera::MoveWithSpherical(const vec2& move)
{
	SetTheta(move.x);
	SetPhi(move.y);

	Vector3 sphericalPos;
	SphericalToCartesian(1.0f, glm::radians(Theta()), glm::radians(Phi()), sphericalPos);
	sphericalPos = glm::normalize(sphericalPos);
	sphericalPos *= LookAtDistance();
	sphericalPos += Center();
	SetLookAt(sphericalPos, Center(), Up());
}

void Camera::SetPhi(float value)
{
	if (m_phi + value >= 179.0f)
	{
		m_phi = 179.0f;
	}
	else if (m_phi + value <= 0.1f)
	{
		m_phi = 0.1f;
	}
	else
	{
		m_phi += value;
	}
}

void Camera::SetTheta(float value)
{
	if (m_theta + value >= 360.0f)
	{
		m_theta = 0.0f;
	}
	else if (m_theta + value <= -360.0f)
	{
		m_theta = 0.0f;
	}
	else
	{
		m_theta += value;
	}
}

void Camera::FitToBDB(const BDB& bdb)
{
	if (!bdb.IsActive()) { return; }
	float lookAtDistance = glm::length(bdb.Max() - bdb.Center()) / (float)sin(m_fov / 2.0);
	lookAtDistance *= 1.2f / m_aspect;

	Vector3 eyeDirection = glm::normalize(Eye() - Center());
	Vector3 newPosition = bdb.Center() + eyeDirection * lookAtDistance;

	SetLookAt(newPosition, bdb.Center(), Up());
}

void Camera::SetAspect(float aspect)
{
	SetPerspective(m_fov, aspect, m_near, m_far);
}
}

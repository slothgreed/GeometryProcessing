#include "Camera.h"
#include "GLBuffer.h"
namespace KI
{
Camera::Camera()
{
	m_theta = 0;
	m_phi = 0;
	m_mode = Mode::NONE;
}

void Camera::SetEye(const Vector3& eye)
{
	SetLookAt(eye, m_center, m_up);
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

void Camera::SetProject(const Matrix4x4& proj)
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
	m_perspective.m_fov = fov;
	m_perspective.m_aspect = aspect;
	m_perspective.m_near = _near;
	m_perspective.m_far = _far;
	m_mode = Mode::PERSPECTIVE_MODE;
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


void Camera::SetOrtho(const Ortho& ortho)
{
	m_ortho = ortho;
	SetProject(glm::ortho(ortho.m_left, ortho.m_right, ortho.m_bottom, ortho.m_top, ortho.m_near, ortho.m_far));
	m_mode = Mode::ORTHO_MODE;
}

String Camera::Ortho::ToString() const
{
	std::ostringstream oss;
	oss << "("
		<< "left,right,top,bottom,near,far"
		<< ") = ("
		<< m_left << "," << m_right << ","
		<< m_top << "," << m_bottom << ","
		<< m_near << "," << m_far
		<< ")";
	return oss.str();
}

Vector3 Camera::ScreenToWorld(const Vector3& pos) const
{
	return glm::unProject(pos, m_View, m_Project, m_viewport);
}

Vector3 Camera::WorldToScreen(const Vector3& world) const
{
	auto clip = m_Project * m_View * Vector4(world, 1.0f);
	auto ndc = glm::vec3(clip) / clip.w;

	float x = m_viewport[0] + (ndc.x + 1.0f) * 0.5f * m_viewport[2];
	float y = m_viewport[1] + (ndc.y + 1.0f) * 0.5f * m_viewport[3];
	float z = (ndc.z + 1.0f) * 0.5f;

	return Vector3(x, y, z);
}

Ray Camera::CreateRay(const Vector2& screen) const
{
	auto nearPoint = glm::unProject(
		Vector3(screen.x, m_viewport[3] - screen.y, 0.0f),
		m_View, m_Project, m_viewport
	);

	auto farPoint = glm::unProject(
		Vector3(screen.x, m_viewport[3] - screen.y, 1.0f),
		m_View, m_Project, m_viewport
	);

	return Ray(nearPoint, glm::normalize(farPoint - nearPoint));
}

Camera::Frustum Camera::CreateFrustum() const
{
	auto VP = m_Project * m_View;
	Camera::Frustum frustm{};
	// Left
	frustm.plane[0].x = VP[0][3] + VP[0][0];
	frustm.plane[0].y = VP[1][3] + VP[1][0];
	frustm.plane[0].z = VP[2][3] + VP[2][0];
	frustm.plane[0].w = VP[3][3] + VP[3][0];
	frustm.plane[0] /= glm::length(Vector3(frustm.plane[0]));

	// Right
	frustm.plane[1].x = VP[0][3] - VP[0][0];
	frustm.plane[1].y = VP[1][3] - VP[1][0];
	frustm.plane[1].z = VP[2][3] - VP[2][0];
	frustm.plane[1].w = VP[3][3] - VP[3][0];
	frustm.plane[1] /= glm::length(Vector3(frustm.plane[1]));

	// Bottom
	frustm.plane[2].x = VP[0][3] + VP[0][1];
	frustm.plane[2].y = VP[1][3] + VP[1][1];
	frustm.plane[2].z = VP[2][3] + VP[2][1];
	frustm.plane[2].w = VP[3][3] + VP[3][1];
	frustm.plane[2] /= glm::length(Vector3(frustm.plane[2]));

	// Top
	frustm.plane[3].x = VP[0][3] - VP[0][1];
	frustm.plane[3].y = VP[1][3] - VP[1][1];
	frustm.plane[3].z = VP[2][3] - VP[2][1];
	frustm.plane[3].w = VP[3][3] - VP[3][1];
	frustm.plane[3] /= glm::length(Vector3(frustm.plane[3]));

	// Near
	frustm.plane[4].x = VP[0][3] + VP[0][2];
	frustm.plane[4].y = VP[1][3] + VP[1][2];
	frustm.plane[4].z = VP[2][3] + VP[2][2];
	frustm.plane[4].w = VP[3][3] + VP[3][2];
	frustm.plane[4] /= glm::length(Vector3(frustm.plane[4]));

	// Far
	frustm.plane[5].x = VP[0][3] - VP[0][2];
	frustm.plane[5].y = VP[1][3] - VP[1][2];
	frustm.plane[5].z = VP[2][3] - VP[2][2];
	frustm.plane[5].w = VP[3][3] - VP[3][2];
	frustm.plane[5] /= glm::length(Vector3(frustm.plane[5]));

	return frustm;
}
Vector2 Camera::GetOnePixelDistance(const Vector3& worldPos) const
{
	Vector3 screen = WorldToScreen(worldPos);
	auto dx = ScreenToWorld(Vector3(screen.x + 1.0f, screen.y, screen.z));
	auto dy = ScreenToWorld(Vector3(screen.x, screen.y + 1.0f, screen.z));
	return Vector2(
		glm::length(dx - worldPos),
		glm::length(dy - worldPos));
}


}

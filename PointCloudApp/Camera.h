#ifndef CAMERA_H
#define	CAMERA_H
#include "BDB.h"
namespace KI
{
class GLBuffer;
class Camera
{
public:
	Camera();
	~Camera() {};

	void SetPerspective(float fov, float aspect, float _near, float _far);
	void MoveWithSpherical(const vec2& move);
	const float Phi() const { return m_phi; }
	const float Theta() const { return m_theta; }

	void SetLookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
	void SetProject(const Matrix4x4 proj);
	const Matrix4x4& ViewMatrix() const { return m_View; }
	const Matrix4x4& Projection() const { return m_Project; }

	Vector3 XDirection();
	Vector3 YDirection();
	Vector3 ZDirection();
	const Vector3& Eye() { return m_eye; };
	const Vector3& Center() { return m_center; };
	const Vector3& Up() { return m_up; };
	const Vector3& Direction() { return m_direction; }

	const float LookAtDistance() const { return m_distance; }

	void FitToBDB(const BDB& bdb);

	void SetAspect(float aspect);

private:
	void SphericalToCartesian(float radius, float x, float y, Vector3& result);
	void SetTheta(float value);
	void SetPhi(float value);

	float m_fov;
	float m_aspect;
	float m_near;
	float m_far;

	float m_theta;
	float m_phi;

	Matrix4x4 m_View;
	Matrix4x4 m_Project;

	Vector3 m_eye;
	Vector3 m_center;
	Vector3 m_up;
	Vector3 m_direction;


	float m_distance;
};
}
#endif // CAMERA_H
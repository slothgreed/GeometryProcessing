#ifndef CAMERA_H
#define	CAMERA_H
#include "BDB.h"

class Camera
{
public:
	Camera();
	~Camera() {};

	void SetPerspective(float fov, float aspect, float _near, float _far);
	void MoveWithSpherical(const vec2& move);
	const float Phi() const { return m_phi; }
	const float Theta() const { return m_theta; }

	void SetLookAt(const vec3& eye, const vec3& center, const vec3& up);
	void SetProject(const mat4x4 proj);
	const mat4x4& ViewMatrix() const { return m_View; }
	const mat4x4& Projection() const { return m_Project; }

	vec3 XDirection();
	vec3 YDirection();
	vec3 ZDirection();
	const vec3& Eye() { return m_eye; };
	const vec3& Center() { return m_center; };
	const vec3& Up() { return m_up; };
	const vec3& Direction() { return m_direction; }

	const float LookAtDistance() const { return m_distance; }

	void FitToBDB(const BDB& bdb);

	void SetAspect(float aspect);
private:
	void SphericalToCartesian(float radius, float x, float y, vec3& result);
	void SetTheta(float value);
	void SetPhi(float value);

	float m_fov;
	float m_aspect;
	float m_near;
	float m_far;

	float m_theta;
	float m_phi;

	mat4x4 m_View;
	mat4x4 m_Project;

	vec3 m_eye;
	vec3 m_center;
	vec3 m_up;
	vec3 m_direction;


	float m_distance;
};
#endif // CAMERA_H
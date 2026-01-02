#ifndef CAMERA_H
#define	CAMERA_H
#include "BDB.h"
#include "KIMath.h"
namespace KI
{
class GLBuffer;
class Camera
{
public:
	Camera();
	~Camera() {};


	struct Perspective
	{
		Perspective()
			: m_fov(0.0f)
			, m_aspect(0.0f)
			, m_near(0.0f)
			, m_far(0.0f)
		{

		}

		float m_fov;
		float m_aspect;
		float m_near;
		float m_far;

	};
	struct Ortho
	{
		Ortho()
			: m_left(0.0f), m_right(0.0f)
			, m_top(0.0f), m_bottom(0.0f)
			, m_near(0.0f), m_far(0.0f)
		{

		}
		Ortho(float l, float r, float t, float b, float n, float f)
			: m_left(l), m_right(r)
			, m_top(t), m_bottom(b)
			, m_near(n), m_far(f)
		{
		}

		Vector2 Center() const { return Vector2(m_left + m_right, m_top + m_bottom) * 0.5f; }
		float Width() const { return std::abs(m_left - m_right); }
		float Height() const { return std::abs(m_top - m_bottom); }
		float m_left;
		float m_right;
		float m_top;
		float m_bottom;
		float m_near;
		float m_far;
		String ToString() const;
	};

	void SetPerspective(float fov, float aspect, float _near, float _far);
	void MoveWithSpherical(const vec2& move);
	float Phi() const { return m_phi; }
	float Theta() const { return m_theta; }
	float FOV() const { return m_perspective.m_fov; }
	void SetEye(const Vector3& eye);
	void SetOrtho(const Ortho& ortho);
	void SetLookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
	void SetProject(const Matrix4x4& proj);
	void SetViewport(const Vector4i& viewport) { m_viewport = viewport; }
	const Matrix4x4& ViewMatrix() const { return m_View; }
	const Matrix4x4& Projection() const { return m_Project; }
	bool IsOrtho() const { return m_mode == Mode::ORTHO_MODE; }
	bool IsPerspective() const { return m_mode == Mode::PERSPECTIVE_MODE; }
	Vector3 XDirection();
	Vector3 YDirection();
	Vector3 ZDirection();
	float Aspect() const { auto viewSize = ViewSize(); return viewSize.x / (float)viewSize.y; }
	Vector2i ViewSize() const { return Vector2(m_viewport.z - m_viewport.x, m_viewport.w - m_viewport.y); }
	const Vector3& Eye() const { return m_eye; };
	const Vector3& Center() const { return m_center; };
	const Vector3& Up() const { return m_up; };
	const Vector3& Direction() const { return m_direction; }

	const float LookAtDistance() const { return m_distance; }

	const Perspective& GetPerspective() const { return m_perspective; }
	const Ortho& GetOrtho() const { return m_ortho; }
	float Left() const { return m_ortho.m_left; }
	float Right() const { return m_ortho.m_right; }
	float Top() const { return m_ortho.m_top; }
	float Bottom() const { return m_ortho.m_bottom; }
	const Matrix4x4& GetViewProj() const { return m_View * m_Project; }
	Vector3 ScreenToWorld(const Vector3& pos) const;
	Vector3 WorldToScreen(const Vector3& world) const;
	Vector2 GetOnePixelDistance(const Vector3& worldPos) const;
	Ray CreateRay(const Vector2& screen) const;
private:
	enum Mode
	{
		NONE,
		PERSPECTIVE_MODE,
		ORTHO_MODE
	};

	Mode m_mode;

	void SphericalToCartesian(float radius, float x, float y, Vector3& result);
	void SetTheta(float value);
	void SetPhi(float value);

	float m_theta;
	float m_phi;

	Matrix4x4 m_View;
	Matrix4x4 m_Project;

	Vector4i m_viewport;
	Vector3 m_eye;
	Vector3 m_center;
	Vector3 m_up;
	Vector3 m_direction;

	Ortho m_ortho;
	Perspective m_perspective;
	float m_distance;
};
}
#endif // CAMERA_H
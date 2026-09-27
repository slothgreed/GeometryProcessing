#ifndef CAMERA_FRUSTUM_DEBUG_H
#define CAMERA_FRUSTUM_DEBUG_H
#include "Camera.h"
#include "GLBuffer.h"

namespace KI
{
struct DrawContext;

// The captured camera defines the geometry; DrawContext selects the viewing camera.
class CameraFrustumDebug
{
public:
	bool ShowUI(Camera& camera, const Vector3i& clusterPartition);
	void Draw(const DrawContext& context);
private:
	void Capture(const Camera& camera, const Vector3i& clusterPartition);
	void BuildGeometry();
	Camera m_camera;
	Vector3i m_partition = Vector3i(16);
	bool m_visible = false;
	bool m_captured = false;
	bool m_showClusters = false;
	bool m_dirty = false;
	GLBuffer m_outline;
	GLBuffer m_grid;
};
}
#endif

#include "Utility/CameraFrustumDebug.h"
#include "Node/RenderNode.h"
#include "Renderer/Shader/SimpleShader.h"
#include "Utility/Utility.h"

namespace KI
{
void CameraFrustumDebug::Capture(const Camera& camera, const Vector3i& clusterPartition)
{
	m_camera = camera;
	m_partition = clusterPartition;
	m_captured = true;
	m_dirty = true;
}

bool CameraFrustumDebug::ShowUI(Camera& camera, const Vector3i& clusterPartition)
{
	bool applied = false;
	if (ImGui::TreeNode("Camera Frustum")) {
		if (ImGui::Checkbox("Show Frustum", &m_visible) && m_visible && !m_captured) {
			Capture(camera, clusterPartition);
		}
		if (ImGui::Button("Capture Current Camera")) {
			Capture(camera, clusterPartition);
			m_visible = true;
		}
		if (m_captured && ImGui::Button("Apply Captured Camera")) {
			const auto viewport = camera.GetViewport();
			camera = m_camera;
			camera.SetViewport(viewport);
			// Keep the current render dimensions after a window resize.
			if (camera.IsPerspective() && viewport.z > 0 && viewport.w > 0) {
				camera.SetPerspective(camera.FOV(), float(viewport.z) / viewport.w,
					camera.GetNear(), camera.GetFar());
			}
			applied = true;
		}
		if (ImGui::Checkbox("Show Z Slice Boundaries", &m_showClusters)) { m_dirty = true; }
		ImGui::TextUnformatted("Capture, then move the camera to inspect from outside.");
		if (m_captured) {
			ImGui::Text("Z slices: %d", m_partition.z);
			ImGui::Text("Depth: %.3f - %.3f (linear)", m_camera.GetNear(), m_camera.GetFar());
		}
		ImGui::TreePop();
	}
	return applied;
}

void CameraFrustumDebug::BuildGeometry()
{
	const auto size = m_camera.ViewSize();
	if (size.x <= 0 || size.y <= 0 || m_partition.x <= 0 || m_partition.y <= 0 || m_partition.z <= 0) { return; }
	const auto invProj = glm::inverse(m_camera.Projection());
	const auto invView = glm::inverse(m_camera.ViewMatrix());
	// Interpolating unprojected endpoints in view space matches the culler's
	// linear depth slices and also works with orthographic cameras.
	auto point = [&](float x, float y, float t) {
		Vector4 a = invProj * Vector4(x, y, -1.0f, 1.0f);
		Vector4 b = invProj * Vector4(x, y, 1.0f, 1.0f);
		a /= a.w;
		b /= b.w;
		return Vector3(invView * (a * (1.0f - t) + b * t));
	};
	Vector<Vector3> outline, grid;
	auto line = [](Vector<Vector3>& vertices, const Vector3& a, const Vector3& b) {
		vertices.push_back(a);
		vertices.push_back(b);
	};
	for (int z = 0; z <= 1; ++z) {
		line(outline, point(-1, -1, float(z)), point(1, -1, float(z)));
		line(outline, point(1, -1, float(z)), point(1, 1, float(z)));
		line(outline, point(1, 1, float(z)), point(-1, 1, float(z)));
		line(outline, point(-1, 1, float(z)), point(-1, -1, float(z)));
	}
	for (int y : {-1, 1}) for (int x : {-1, 1}) {
		line(outline, point(float(x), float(y), 0), point(float(x), float(y), 1));
	}
	if (m_showClusters) {
		// Near and far boundaries are already part of the outline.
		for (int z = 1; z < m_partition.z; ++z) {
			const float t = float(z) / m_partition.z;
			line(grid, point(-1, -1, t), point(1, -1, t));
			line(grid, point(1, -1, t), point(1, 1, t));
			line(grid, point(1, 1, t), point(-1, 1, t));
			line(grid, point(-1, 1, t), point(-1, -1, t));
		}
	}
	m_outline.Create(outline);
	if (!grid.empty()) { m_grid.Create(grid); }
	else { m_grid.Delete(); }
	m_dirty = false;
}

void CameraFrustumDebug::Draw(const DrawContext& context)
{
	if (!m_visible || !m_captured) { return; }
	if (m_dirty) { BuildGeometry(); }
	if (!m_outline.Created()) { return; }
	auto shader = context.pResource->GetShaderTable()->GetSimpleShader();
	shader->Use();
	shader->SetCamera(context.pResource->GetCameraBuffer());
	shader->SetModel(Matrix4x4(1.0f));
	if (m_showClusters && m_grid.Created()) {
		shader->SetPosition(&m_grid);
		// Each internal Z boundary contains four lines (eight vertices).
		for (int i = 0; i < m_partition.z - 1; ++i) {
			shader->SetColor(ColorUtility::CreatePrimary(i % 8));
			shader->DrawArray(GL_LINES, i * 8, 8);
		}
	}
	shader->SetColor(Vector3(1.0f, 0.65f, 0.1f));
	shader->SetPosition(&m_outline);
	shader->DrawArray(GL_LINES, m_outline.Num());
}
}

#ifndef KI_STEP_VIEWER_H
#define KI_STEP_VIEWER_H
#include "App/GLFWApp.h"
#include "Node/RenderNode.h"
#include <string>

namespace KI
{
class STEPRenderNode;

class STEPViewer : public GLFWApp
{
public:
	STEPViewer() = default;
	void Initialize() override;
	void Execute() override;
	void Finalize() override;
	void ProcessMouseEvent(const MouseInput& input) override;
	void ResizeEvent(int width, int height) override;

private:
	void InitializeFiles();
	void UpdateRenderData();
	Shared<STEPRenderNode> LoadSTEP(int index, bool reload = false);
	void DrawGallery(const DrawContext& context);
	void FitCamera();
	void ShowUI();
	Vector<String> m_files;
	Vector<Shared<STEPRenderNode>> m_nodes;
	Vector<String> m_status;
	Vector<bool> m_loadAttempted;
	int m_folder = 6; // finish (CreateSTEPNodeTest)
	int m_selectedFile = 0;
	bool m_galleryMode = false;
	int m_galleryPage = 0;
	static constexpr int GalleryColumns = 4;
	static constexpr int GalleryRows = 3;
	static constexpr int GalleryPageSize = GalleryColumns * GalleryRows;
	bool m_animation = false;
	float m_elapsed = 0.0f;
	Shared<RenderResource> m_pResource;
	Shared<STEPRenderNode> m_pSTEPNode;
	Shared<RenderNode> m_pDebugRoot;
	UIContext m_uiContext;
};
}
#endif // KI_STEP_VIEWER_H

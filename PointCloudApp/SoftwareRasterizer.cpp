#include "SoftwareRasterizer.h"
#include "Texture.h"
#include "PostEffect.h"
#include "Utility.h"
#include "GeometryUtility.h"
namespace KI
{
void SoftwareRasterizer::Initialize()
{
	GLFWApp::Initialize();
}

void SoftwareRasterizer::Execute()
{
	GLuint VertexArrayID;
	auto m_pResource = std::make_unique<RenderResource>();
	m_pResource->Build();
	DrawContext context(m_pResource.get());

	auto pTexture = std::make_unique<Texture2D>();
	Vector<Vector3> position;
	position.push_back(Vector3(-0.5f, -0.5f, 0));
	position.push_back(Vector3(0.5f, -0.5f, 0));
	position.push_back(Vector3(0.0f, 0.5f, 0));
	m_mesh = Mesh(std::move(position), Mesh::DrawType::Triangles);
	auto viewport = Viewport(m_windowSize);
	m_pCameraController->FitToBDB(BDB(m_mesh.GetPoints()));
	Matrix4x4 vp = Matrix4x4(1.0);
	Matrix4x4 model = Matrix4x4(1.0);
	PixelData pixelData;
	pixelData.Allocate(m_windowSize.x, m_windowSize.y, 4);
	for (size_t k = 0; k < m_mesh.TriangleNum(); k++) {
		auto tri = m_mesh.GetTriangle(k);
		Vector3 screen0;
		Vector3 screen1;
		Vector3 screen2;
		if (!MathHelper::ToScreen(viewport.Get(), vp, model, tri.p0, screen0)) { continue; }
		if (!MathHelper::ToScreen(viewport.Get(), vp, model, tri.p1, screen1)) { continue; }
		if (!MathHelper::ToScreen(viewport.Get(), vp, model, tri.p2, screen2)) { continue; }

		Rangef xRange, yRange;
		xRange.Add(screen0.x); xRange.Add(screen1.x); xRange.Add(screen2.x);
		yRange.Add(screen0.y); yRange.Add(screen1.y); yRange.Add(screen2.y);

		int minX = std::max(0, xRange.RoundMin());
		int maxX = std::min(m_windowSize.x - 1, xRange.RoundMax());
		int minY = std::max(0, yRange.RoundMin());
		int maxY = std::min(m_windowSize.y - 1, yRange.RoundMax());

		Vector2 v0{ screen0.x, screen0.y };
		Vector2 v1{ screen1.x, screen1.y };
		Vector2 v2{ screen2.x, screen2.y };

		for (int i = minX; i < maxX; i++) {
			for (int j = minY; j < maxY; j++) {
				Vector2 p{ i + 0.5f, j + 0.5f };
				if (GeometryUtility::InTriangle(screen0, screen1, screen2, p)) {
					pixelData.Set(i, j, Vector4(255.0f, 255.0f, 0.0f, 255.0f));
				}
			}
		}
	}

	pTexture->Build(pixelData.width, pixelData.height, pixelData.data);
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	while (glfwWindowShouldClose(m_window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		TextureDrawer::Execute(context, pTexture.get());
		glfwSwapBuffers(m_window);

		glfwWaitEvents();
		OUTPUT_GLERROR;
	}
	glDeleteVertexArrays(1, &VertexArrayID);
}
void SoftwareRasterizer::Finalize()
{
	GLFWApp::Finalize();
}
}
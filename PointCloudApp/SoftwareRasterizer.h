#ifndef SOFTWARE_RASTERIZER_H
#include "GLFWApp.h"
#include "Mesh.h"
#include "Texture.h"
namespace KI
{
class HalfEdgeStruct;
class SoftwareRasterizer : public GLFWApp
{
public:
	SoftwareRasterizer() {};
	~SoftwareRasterizer() {};

	virtual void Initialize();
	virtual void Execute();
	virtual void Finalize();

	virtual void ProcessMouseEvent(const MouseInput& input);
	virtual void ResizeEvent(int width, int height);

private:
	void Printf(int x, int y);
	struct UI
	{
		bool useTBB = false;
		int tbbGrainSize = 0;
		bool pickMode = false;
		bool showDepth = false;
	};

	UI m_ui;
	void Rasterize();
	//bool IsDrawTriangle(const Vector3& screen0, const Vector3& screen1, const Vector3& screen2, float area, const Vector2& target, float& targetZ);

	PixelDataf m_colorPixel;
	PixelDataf m_depthPixel;
	PixelDataf m_debugPixel; // triangle;
	std::shared_ptr<Texture2D> m_pColorTexture = nullptr;
	std::shared_ptr<Texture2D> m_pDepthTexture = nullptr;
	std::shared_ptr<HalfEdgeStruct> m_pBunny = nullptr;
};

}

#endif // !SOFTWARE_RASTERIZER_H

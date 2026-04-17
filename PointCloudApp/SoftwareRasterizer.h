#ifndef SOFTWARE_RASTERIZER_H
#include "GLFWApp.h"
#include "Mesh.h"
namespace KI
{
class SoftwareRasterizer : public GLFWApp
{
public:
	SoftwareRasterizer() {};
	~SoftwareRasterizer() {};

	virtual void Initialize();
	virtual void Execute();
	virtual void Finalize();
private:
	Mesh m_mesh;
};

}

#endif // !SOFTWARE_RASTERIZER_H

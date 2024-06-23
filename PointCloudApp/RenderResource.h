#ifndef RENDER_RESOURCE_H
#define RENDER_RESOURCE_H
#include "ShaderTable.h"

namespace KI
{
class Camera;
class RenderResource
{
public:
	RenderResource()
		: m_pCamera(nullptr){};
	~RenderResource() {};
	void Build();

	void Finalize();
	ShaderTable* GetShaderTable() { return &m_pShaderTable; };
	GLBuffer* GetCameraBuffer() { return m_pCamera; }

	void UpdateCamera(const Camera* pCamera);
private:
	GLBuffer* m_pCamera;
	ShaderTable m_pShaderTable;
};
}

#endif RENDER_RESOURCE_H
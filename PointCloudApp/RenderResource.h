#ifndef RENDER_RESOURCE_H
#define RENDER_RESOURCE_H
#include "ShaderTable.h"

namespace KI
{

class RenderResource
{
public:
	RenderResource() {};
	~RenderResource() {};
	void Build() {
		m_pShaderTable.Build();
	};
	ShaderTable* GetShaderTable() { return &m_pShaderTable; };
private:
	ShaderTable m_pShaderTable;
};
}

#endif RENDER_RESOURCE_H
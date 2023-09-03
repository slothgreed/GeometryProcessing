#ifndef RENDER_RESOURCE_H
#define RENDER_RESOURCE_H
#include "ShaderTable.h"

class RenderResource
{
public:
	RenderResource() {};
	~RenderResource() {};
	void Build() {
		m_pShaderTable.Build();
	};
	shared_ptr<IShader> GetShader(IShader::Type type) { return m_pShaderTable.Get(type); };
private:
	ShaderTable m_pShaderTable;
};

#endif RENDER_RESOURCE_H
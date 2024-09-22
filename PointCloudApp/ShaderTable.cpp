#include "ShaderTable.h"
#include "SimpleShader.h"
namespace KI
{

void ShaderTable::Build()
{
	m_pSimpleShader = std::make_shared<SimpleShader>();
	m_pSimpleShader->Build();

	m_pVertexColorShader = std::make_shared<VertexColorShader>();
	m_pVertexColorShader->Build();

	m_pTextureShader = std::make_shared<TextureShader>();
	m_pTextureShader->Build();

	m_pPrimitiveColorShader = std::make_shared<PrimitiveColorShader>();
	m_pPrimitiveColorShader->Build();

	m_pInstancedShader = std::make_shared<InstancedPrimitiveShader>();
	m_pInstancedShader->Build();
}

}
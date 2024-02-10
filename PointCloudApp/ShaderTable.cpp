#include "ShaderTable.h"
#include "SimpleShader.h"
#include "VertexColorShader.h"
void ShaderTable::Build()
{
	m_pTable.push_back(std::make_shared<SimpleShader>());
	m_pTable.push_back(std::make_shared<VertexColorShader>());
	m_pTable.push_back(std::make_shared<TextureShader>());

	for (auto& pTable : m_pTable) {
		pTable->Build();
	}
}

Shared<IShadingShader> ShaderTable::Get(IShadingShader::Type type)
{
	return m_pTable[(int)type];
}

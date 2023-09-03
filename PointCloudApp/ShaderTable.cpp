#include "ShaderTable.h"
#include "SimpleShader.h"
#include "VertexColorShader.h"
void ShaderTable::Build()
{
	m_pTable.push_back(make_shared<SimpleShader>());
	m_pTable.push_back(make_shared<VertexColorShader>());

	for (auto& pTable : m_pTable) {
		pTable->Build();
	}
}

shared_ptr<IShader> ShaderTable::Get(IShader::Type type)
{
	return m_pTable[(int)type];
}

#ifndef SHADER_TABLE_H
#define SHADER_TABLE_H
#include "IShader.h"
namespace KI
{

class ShaderTable
{
public:
	ShaderTable() {};
	~ShaderTable() {};

	void Build();

	Shared<IShadingShader> Get(IShadingShader::Type type);
	
private:
	Vector<Shared<IShadingShader>> m_pTable;
};
}

#endif SHADER_TABLE_H
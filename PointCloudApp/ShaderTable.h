#ifndef SHADER_TABLE_H
#define SHADER_TABLE_H
#include "IShader.h"
class ShaderTable
{
public:
	ShaderTable() {};
	~ShaderTable() {};

	void Build();

	shared_ptr<IShader> Get(IShader::Type type);
	
private:
	std::vector<shared_ptr<IShader>> m_pTable;
};

#endif SHADER_TABLE_H
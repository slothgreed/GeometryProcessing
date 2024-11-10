#ifndef SHADER_TABLE_H
#define SHADER_TABLE_H
#include "IShader.h"
namespace KI
{
class SimpleShader;
class VertexColorShader;
class TextureShader;
class PrimitiveColorShader;
class InstancedPrimitiveShader;
class PointPickShader;
class ShaderTable
{
public:
	ShaderTable() {};
	~ShaderTable() {};

	void Build();

	const Shared<SimpleShader>& GetSimpleShader() { return m_pSimpleShader; }
	const Shared<VertexColorShader>& GetVertexColorShader() { return m_pVertexColorShader; }
	const Shared<PrimitiveColorShader>& GetPrimitiveColorShader() { return m_pPrimitiveColorShader; }
	const Shared<TextureShader>& GetTextureShader() { return m_pTextureShader; }
	const Shared<InstancedPrimitiveShader>& GetInstancedShader() { return m_pInstancedShader; }
	const Shared<PointPickShader>& GetPointPickShader() { return m_pPointPickShader; }
	const Shared<TextureShader>& GetTextureUINTShader() { return m_pTextureUINTShader; }
private:
	Shared<PointPickShader> m_pPointPickShader;
	Shared<SimpleShader> m_pSimpleShader;
	Shared<VertexColorShader> m_pVertexColorShader;
	Shared<PrimitiveColorShader> m_pPrimitiveColorShader;
	Shared<InstancedPrimitiveShader> m_pInstancedShader;
	Shared<TextureShader> m_pTextureShader;
	Shared<TextureShader> m_pTextureUINTShader;
};
}

#endif SHADER_TABLE_H
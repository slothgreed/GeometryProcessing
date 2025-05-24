#ifndef SHADER_TABLE_H
#define SHADER_TABLE_H
#include "IShader.h"
namespace KI
{
class SimpleShader;
class VertexColorShader;
class TextureShader;
class VertexVectorShader;
class PrimitiveColorShader;
class InstancedPrimitiveShader;
class PointPickShader;
class TextureViewShader;
class ShaderTable
{
public:
	ShaderTable() {};
	~ShaderTable() {};

	void Build();

	const Shared<SimpleShader>& GetSimpleShader() { return m_pSimpleShader; }
	const Shared<VertexVectorShader>& GetVertexVectorShader() { return m_pVertexVectorShader; }
	const Shared<VertexColorShader>& GetVertexColorShader() { return m_pVertexColorShader; }
	const Shared<PrimitiveColorShader>& GetPrimitiveColorShader() { return m_pPrimitiveColorShader; }
	const Shared<TextureShader>& GetTextureShader() { return m_pTextureShader; }
	const Shared<InstancedPrimitiveShader>& GetInstancedShader() { return m_pInstancedShader; }
	const Shared<PointPickShader>& GetPointPickByPrimitive() { return m_pPointPickByPrimitive; }
	const Shared<PointPickShader>& GetPointPickByID() { return m_pPointPickByID; }
	const Shared<TextureShader>& GetTextureUINTShader() { return m_pTextureUINTShader; }
	const Shared<TextureViewShader>& GetTextureViewShader() { return m_pTextureViewShader; }
private:
	Shared<VertexVectorShader> m_pVertexVectorShader;
	Shared<PointPickShader> m_pPointPickByPrimitive;
	Shared<PointPickShader> m_pPointPickByID;
	Shared<SimpleShader> m_pSimpleShader;
	Shared<VertexColorShader> m_pVertexColorShader;
	Shared<PrimitiveColorShader> m_pPrimitiveColorShader;
	Shared<InstancedPrimitiveShader> m_pInstancedShader;
	Shared<TextureShader> m_pTextureShader;
	Shared<TextureShader> m_pTextureUINTShader;
	Shared<TextureViewShader> m_pTextureViewShader;
};
}

#endif SHADER_TABLE_H
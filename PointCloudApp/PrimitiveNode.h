#ifndef PRIMITIVE_NODE_H
#define PRIMITIVE_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Primitive.h"
namespace KI
{

class PrimitiveNode : public RenderNode
{
public:
	PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive, const Vector3& color);
	PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive);
	~PrimitiveNode();

	const Shared<Primitive>& GetData() const;
	void UpdateData();
	void DrawNode(const DrawContext& context);
private:
	void BuildGLBuffer();
	void UpdateRenderData();
	Vector3 m_color;
	Shared<VertexColorShader> m_pVertexColorShader;
	Shared<PrimitiveColorShader> m_pPrimitiveColorShader;
	Shared<SimpleShader> m_pSimpleShader;

	Shared<Primitive> m_pPrimitive;
	Unique<GLBuffer> m_pPositionBuffer;
	Unique<GLBuffer> m_pColorBuffer;
	Unique<GLBuffer> m_pIndexBuffer;
};
}

#endif PRIMITIVE_NODE_H
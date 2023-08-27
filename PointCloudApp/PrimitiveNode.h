#ifndef PRIMITIVE_NODE_H
#define PRIMITIVE_NODE_H
#include "RenderNode.h"
#include "SimpleShader.h"
#include "Primitive.h"
class PrimitiveNode : public RenderNode
{
public:
	PrimitiveNode(std::unique_ptr<Primitive>&& pPrimitive);
	~PrimitiveNode();

	const std::unique_ptr<Primitive>& GetData() const;
	void UpdateData();
	void DrawData(const mat4x4& proj, const mat4x4& view);

private:
	void BuildGLBuffer();
	void UpdateRenderData();
	std::unique_ptr<SimpleShader> m_pShader;
	std::unique_ptr<Primitive> m_pPrimitive;
	std::unique_ptr<GLBuffer> m_pPositionBuffer;
	std::unique_ptr<GLBuffer> m_pColorBuffer;
};

#endif PRIMITIVE_NODE_H
#ifndef PRIMITIVE_NODE_H
#define PRIMITIVE_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Primitive.h"
class PrimitiveNode : public RenderNode
{
public:
	PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive, const glm::vec3& color);
	PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive);
	~PrimitiveNode();

	const Shared<Primitive>& GetData() const;
	void UpdateData();
	void DrawData(const mat4x4& proj, const mat4x4& view);

private:
	void BuildGLBuffer();
	void UpdateRenderData();
	glm::vec3 m_color;
	Shared<IShadingShader> m_pShader;

	Shared<Primitive> m_pPrimitive;
	std::unique_ptr<GLBuffer> m_pPositionBuffer;
	std::unique_ptr<GLBuffer> m_pColorBuffer;
	std::unique_ptr<GLBuffer> m_pIndexBuffer;
};

#endif PRIMITIVE_NODE_H
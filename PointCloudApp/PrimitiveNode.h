#ifndef PRIMITIVE_NODE_H
#define PRIMITIVE_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Primitive.h"
class PrimitiveNode : public RenderNode
{
public:
	PrimitiveNode(const string& name, std::shared_ptr<Primitive>& pPrimitive, const glm::vec3& color);
	PrimitiveNode(const string& name, std::shared_ptr<Primitive>& pPrimitive);
	~PrimitiveNode();

	const std::shared_ptr<Primitive>& GetData() const;
	void UpdateData();
	void DrawData(const mat4x4& proj, const mat4x4& view);

private:
	void BuildGLBuffer();
	void UpdateRenderData();
	glm::vec3 m_color;
	std::shared_ptr<IShadingShader> m_pShader;

	std::shared_ptr<Primitive> m_pPrimitive;
	std::unique_ptr<GLBuffer> m_pPositionBuffer;
	std::unique_ptr<GLBuffer> m_pColorBuffer;
	std::unique_ptr<GLBuffer> m_pIndexBuffer;
};

#endif PRIMITIVE_NODE_H
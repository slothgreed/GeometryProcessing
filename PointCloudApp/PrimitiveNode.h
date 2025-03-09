#ifndef PRIMITIVE_NODE_H
#define PRIMITIVE_NODE_H
#include "RenderNode.h"
#include "IShader.h"
#include "Texture.h"
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
	void SetGLStatus(const Shared<GLStatus>& status) { m_gl = status; }
private:

	Shared<GLStatus> m_gl;
	void BuildGLBuffer();
	void UpdateRenderData();
	Vector3 m_color;
	Shared<Primitive> m_pPrimitive;
	Unique<GLBuffer> m_pPositionBuffer;
	Unique<GLBuffer> m_pColorBuffer;
	Unique<GLBuffer> m_pIndexBuffer;
};


class InstancedPrimitiveNode : public RenderNode
{
public:
	InstancedPrimitiveNode(const String& name, const Shared<Primitive>& pPrimitive, const Vector3& color);
	~InstancedPrimitiveNode();

	void SetMatrixs(Vector<Matrix4x4>&& matrix);
	const Shared<Primitive>& GetData() const;
	void UpdateData();
	void DrawNode(const DrawContext& context);
private:
	void BuildGLBuffer();
	void UpdateRenderData();
	Vector3 m_color;

	Shared<Primitive> m_pPrimitive;
	Unique<TextureBuffer> m_pMatrixTexture;
	Unique<GLBuffer> m_pMatrixBuffer;
	Unique<GLBuffer> m_pPositionBuffer;
	Unique<GLBuffer> m_pIndexBuffer;

};

}

#endif PRIMITIVE_NODE_H
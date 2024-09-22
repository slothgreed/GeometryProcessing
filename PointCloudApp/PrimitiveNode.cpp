#include "PrimitiveNode.h"
#include "PointCloud.h"
#include "SimpleShader.h"
namespace KI
{

PrimitiveNode::PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive, const Vector3& color)
	: RenderNode(name)
	, m_color(color)
{
	m_pPrimitive = std::move(pPrimitive);
	SetBoundBox(m_pPrimitive->GetBDB());
	BuildGLBuffer();
}

PrimitiveNode::PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive)
	: RenderNode(name)
{
	m_pPrimitive = std::move(pPrimitive);
	BuildGLBuffer();
}


PrimitiveNode::~PrimitiveNode()
{

}

void PrimitiveNode::BuildGLBuffer()
{
	if (!m_pPrimitive->NeedUpdate()) { return; }

	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(m_pPrimitive->Position());

	if (m_pPrimitive->Color().size() != 0) {
		m_pColorBuffer = std::make_unique<GLBuffer>();
		m_pColorBuffer->Create(m_pPrimitive->Color());
	}

	if (m_pPrimitive->Index().size() != 0) {
		m_pIndexBuffer = std::make_unique<GLBuffer>();
		m_pIndexBuffer->Create(m_pPrimitive->Index());
	}
	m_pPrimitive->ClearUpdate();


}
const Shared<Primitive>& PrimitiveNode::GetData() const
{
	return m_pPrimitive;
}
void PrimitiveNode::UpdateData()
{
	m_pPrimitive->Update();
}
void PrimitiveNode::UpdateRenderData()
{
	BuildGLBuffer();
}


void PrimitiveNode::DrawNode(const DrawContext& context)
{
	UpdateRenderData();
	const auto& pResourece = context.pResource;
	IShadingShader* pShader = nullptr;
	if (m_pPrimitive->Color().size() == m_pPrimitive->Position().size()) {
		auto pPrimitiveColorShader = pResourece->GetShaderTable()->GetPrimitiveColorShader();
		pPrimitiveColorShader->Use();
		pPrimitiveColorShader->SetPosition(m_pPositionBuffer.get());
		pPrimitiveColorShader->SetColor(m_pColorBuffer.get());
		pPrimitiveColorShader->SetCamera(pResourece->GetCameraBuffer());
		pPrimitiveColorShader->SetModel(Matrix4x4(1.0f));
		pShader = pPrimitiveColorShader.get();
	} else 	if (m_pPrimitive->Color().size() == m_pPrimitive->Position().size()) {
		auto pVertexColorShader = pResourece->GetShaderTable()->GetVertexColorShader();
		pVertexColorShader->Use();
		pVertexColorShader->SetPosition(m_pPositionBuffer.get());
		pVertexColorShader->SetColor(m_pColorBuffer.get());
		pVertexColorShader->SetCamera(pResourece->GetCameraBuffer());
		pVertexColorShader->SetModel(Matrix4x4(1.0f));
		pShader = pVertexColorShader.get();
	} else {
		auto pSimpleShader = pResourece->GetShaderTable()->GetSimpleShader();
		pSimpleShader->Use();
		pSimpleShader->SetPosition(m_pPositionBuffer.get());
		pSimpleShader->SetCamera(pResourece->GetCameraBuffer());
		pSimpleShader->SetModel(Matrix4x4(1.0f));
		pSimpleShader->SetColor(m_color);
		pShader = pSimpleShader.get();
	}

	if (m_pIndexBuffer) {
		pShader->DrawElement(m_pPrimitive->GetType(), m_pIndexBuffer.get());
	} else {
		pShader->DrawArray(m_pPrimitive->GetType(), m_pPositionBuffer.get());
	}
}



InstancedPrimitiveNode::InstancedPrimitiveNode(const String& name, const Shared<Primitive>& pPrimitive, const Vector3& color)
	: RenderNode(name)
	, m_pPrimitive(std::move(pPrimitive))
	, m_color(color)
{
	BuildGLBuffer();
}


InstancedPrimitiveNode::~InstancedPrimitiveNode()
{

}

void InstancedPrimitiveNode::SetMatrixs(Vector<Matrix4x4>&& matrix)
{
	if (!m_pMatrixBuffer) {
		m_pMatrixBuffer = std::make_unique<GLBuffer>();
		m_pMatrixBuffer->Create(matrix);
		m_pMatrixTexture = std::make_unique<TextureBuffer>();
		m_pMatrixTexture->Bind(m_pMatrixBuffer->Handle());
	} else {
		m_pMatrixBuffer->BufferSubData(0, matrix);
	}
}
void InstancedPrimitiveNode::BuildGLBuffer()
{
	if (!m_pPrimitive->NeedUpdate()) { return; }

	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(m_pPrimitive->Position());

	if (m_pPrimitive->Index().size() != 0) {
		m_pIndexBuffer = std::make_unique<GLBuffer>();
		m_pIndexBuffer->Create(m_pPrimitive->Index());
	}
	m_pPrimitive->ClearUpdate();


}
const Shared<Primitive>& InstancedPrimitiveNode::GetData() const
{
	return m_pPrimitive;
}
void InstancedPrimitiveNode::UpdateData()
{
	m_pPrimitive->Update();
}
void InstancedPrimitiveNode::UpdateRenderData()
{
	BuildGLBuffer();
}


void InstancedPrimitiveNode::DrawNode(const DrawContext& context)
{
	UpdateRenderData();
	const auto& pResourece = context.pResource;
	IShadingShader* pShader = nullptr;
	auto pInstancedShader = pResourece->GetShaderTable()->GetInstancedShader();
	pInstancedShader->Use();
	pInstancedShader->SetPosition(m_pPositionBuffer.get());
	pInstancedShader->SetCamera(pResourece->GetCameraBuffer());
	pInstancedShader->SetColor(m_color);
	pInstancedShader->SetMatrixTexture(m_pMatrixTexture.get());
	pShader = pInstancedShader.get();

	if (m_pIndexBuffer) {
		pShader->DrawElementInstaced(m_pPrimitive->GetType(), m_pIndexBuffer.get(), m_pMatrixBuffer->Num());
	} else {
		assert(0);
	}

}



}
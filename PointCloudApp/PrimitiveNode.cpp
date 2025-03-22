#include "PrimitiveNode.h"
#include "PointCloud.h"
#include "SimpleShader.h"
#include "Utility.h"
namespace KI
{

PrimitiveNode::PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive, const Vector3& color)
	: RenderNode(name)
	, m_color(color)
	, m_pickTarget(false)
{
	m_pPrimitive = pPrimitive;
	SetBoundBox(m_pPrimitive->GetBDB());
	BuildGLBuffer();
}

PrimitiveNode::PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive, const Shared<Texture>& pTexutre)
	: RenderNode(name)
	, m_pTexture(pTexutre)
	, m_pickTarget(false)
{
	m_pPrimitive = pPrimitive;
	SetBoundBox(m_pPrimitive->GetBDB());
	BuildGLBuffer();
}

PrimitiveNode::PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive)
	: RenderNode(name)
	, m_pickTarget(false)
{
	m_pPrimitive = pPrimitive;
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

	if (m_pPrimitive->Texcoord().size() != 0) {
		m_pTexcoordBuffer = std::make_unique<GLBuffer>();
		m_pTexcoordBuffer->Create(m_pPrimitive->Texcoord());
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

String PrimitiveNode::Parts::ToString()
{
	return glmUtil::ToString(position);
}

void PrimitiveNode::PickNode(const PickContext& context)
{
	if (!m_pickTarget) { return; }
	auto pResource = context.pResource;
	auto pPickShader = pResource->GetShaderTable()->GetPointPickByID();
	pPickShader->Use();
	pPickShader->SetCamera(pResource->GetCameraBuffer());
	pPickShader->SetPosition(m_pPositionBuffer.get());
	pPickShader->SetModel(GetMatrix());
	pPickShader->SetPickOffset(0);
	if (m_pIndexBuffer) {
		pPickShader->DrawElement(m_pPrimitive->GetType(), m_pIndexBuffer.get());
	} else {
		pPickShader->DrawArray(m_pPrimitive->GetType(), m_pPositionBuffer.get());
	}
}

bool PrimitiveNode::CollectPickedNode(PickResult& result)
{
	if (result.id == 0) {
		result.pResult[this] = std::make_unique<PrimitiveNode::Parts>();
		return true;
	}

	return false;
}

void PrimitiveNode::DrawPartsNode(const DrawContext& context, const RenderParts& parts)
{

}

void PrimitiveNode::DrawNode(const DrawContext& context)
{
	if (m_pPrimitive->Position().size() == 0) {
		return;
	}
	UpdateRenderData();
	if (m_gl) {
		context.pResource->GL()->SetupStatus(*m_gl.get());
	}
	const auto& pResourece = context.pResource;
	IShadingShader* pShader = nullptr;
	if (m_pPrimitive->Color().size() * GLUtil::GetPrimitiveSize(m_pPrimitive->GetType()) == m_pPrimitive->Position().size()) {
		auto pPrimitiveColorShader = pResourece->GetShaderTable()->GetPrimitiveColorShader();
		pPrimitiveColorShader->Use();
		pPrimitiveColorShader->SetPosition(m_pPositionBuffer.get());
		pPrimitiveColorShader->SetColor(m_pColorBuffer.get());
		pPrimitiveColorShader->SetCamera(pResourece->GetCameraBuffer());
		pPrimitiveColorShader->SetModel(GetMatrix());
		pShader = pPrimitiveColorShader.get();
	} else 	if (m_pPrimitive->Color().size() == m_pPrimitive->Position().size()) {
		auto pVertexColorShader = pResourece->GetShaderTable()->GetVertexColorShader();
		pVertexColorShader->Use();
		pVertexColorShader->SetPosition(m_pPositionBuffer.get());
		pVertexColorShader->SetColor(m_pColorBuffer.get());
		pVertexColorShader->SetCamera(pResourece->GetCameraBuffer());
		pVertexColorShader->SetModel(GetMatrix());
		pShader = pVertexColorShader.get();
	} else if (m_pPrimitive->Texcoord().size() != 0) {
		auto pTextureShader = pResourece->GetShaderTable()->GetTextureShader();
		pTextureShader->Use();
		pTextureShader->BindTexture(*m_pTexture);
		pTextureShader->SetPosition(m_pPositionBuffer.get());
		pTextureShader->SetTexcoord(m_pTexcoordBuffer.get());
		pTextureShader->SetCamera(pResourece->GetCameraBuffer());
		pTextureShader->SetModel(GetMatrix());
		pShader = pTextureShader.get();
	} else {
		auto pSimpleShader = pResourece->GetShaderTable()->GetSimpleShader();
		pSimpleShader->Use();
		pSimpleShader->SetPosition(m_pPositionBuffer.get());
		pSimpleShader->SetCamera(pResourece->GetCameraBuffer());
		pSimpleShader->SetModel(GetMatrix());
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
	, m_pPrimitive(pPrimitive)
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
		pShader->DrawArrayInstaced(m_pPrimitive->GetType(), m_pPositionBuffer->Num(), m_pMatrixBuffer->Num());
	}

}



}
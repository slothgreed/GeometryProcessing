#include "PrimitiveNode.h"
#include "PointCloud.h"
#include "SimpleShader.h"
namespace KI
{

PrimitiveNode::PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive, const Vector3& color)
	: RenderNode(name)
	, m_color(color)
	, m_pSimpleShader(nullptr)
	, m_pVertexColorShader(nullptr)
	, m_pPrimitiveColorShader(nullptr)
{
	m_pSimpleShader = GetResource()->GetShaderTable()->GetSimpleShader();
	m_pPrimitive = std::move(pPrimitive);
	SetBoundBox(m_pPrimitive->GetBDB());
	BuildGLBuffer();
}

PrimitiveNode::PrimitiveNode(const String& name, Shared<Primitive>& pPrimitive)
	: RenderNode(name)
	, m_pSimpleShader(nullptr)
	, m_pVertexColorShader(nullptr)
	, m_pPrimitiveColorShader(nullptr)
{
	if (pPrimitive->Color().size() == pPrimitive->Position().size()) {
		m_pVertexColorShader = GetResource()->GetShaderTable()->GetVertexColorShader();
	} else if (pPrimitive->Color().size() == pPrimitive->GetTriangleNum()) {
		m_pPrimitiveColorShader = GetResource()->GetShaderTable()->GetPrimitiveColorShader();
	}

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
	IShadingShader* pShader = nullptr;
	if (m_pSimpleShader) {
		m_pSimpleShader->Use();
		m_pSimpleShader->SetPosition(m_pPositionBuffer.get());
		m_pSimpleShader->SetViewProj(context.m_pCamera->Projection() * context.m_pCamera->ViewMatrix());
		m_pSimpleShader->SetModel(Matrix4x4(1.0f));
		m_pSimpleShader->SetColor(m_color);
		pShader = m_pSimpleShader.get();
	} else 	if (m_pVertexColorShader) {
		m_pVertexColorShader->Use();
		m_pVertexColorShader->SetPosition(m_pPositionBuffer.get());
		m_pVertexColorShader->SetColor(m_pColorBuffer.get());
		m_pVertexColorShader->SetViewProj(context.m_pCamera->Projection() * context.m_pCamera->ViewMatrix());
		m_pVertexColorShader->SetModel(Matrix4x4(1.0f));
		pShader = m_pVertexColorShader.get();
	} else if (m_pPrimitiveColorShader) {
		m_pPrimitiveColorShader->Use();
		m_pPrimitiveColorShader->SetPosition(m_pPositionBuffer.get());
		m_pPrimitiveColorShader->SetColor(m_pColorBuffer.get());
		m_pPrimitiveColorShader->SetViewProj(context.m_pCamera->Projection() * context.m_pCamera->ViewMatrix());
		m_pPrimitiveColorShader->SetModel(Matrix4x4(1.0f));
		pShader = m_pPrimitiveColorShader.get();
	} else {
		assert(0);
	}

	if (m_pIndexBuffer) {
		pShader->DrawElement(m_pPrimitive->GetType(), m_pIndexBuffer.get());
	} else {
		pShader->DrawArray(m_pPrimitive->GetType(), m_pPositionBuffer.get());
	}
}
}
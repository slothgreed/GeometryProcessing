#include "RenderTextureNode.h"
namespace KI
{

RenderTextureNode::RenderTextureNode(const String& name, const Shared<Texture>& pTexture)
	:RenderNode(name)
	,m_pTexture(pTexture)
{
}


void RenderTextureNode::DrawNode(const DrawContext& context)
{
	BuildGLBuffer();
	Shared<TextureShader> pShader = nullptr;
	if (m_pTexture->GetFormat().format == GL_RED_INTEGER) {
		pShader = context.pResource->GetShaderTable()->GetTextureUINTShader();
	} else {
		pShader = context.pResource->GetShaderTable()->GetTextureShader();

	}
	//glEnable(GL_TEXTURE_2D);
	//OUTPUT_GLERROR;
	pShader->Use();
	pShader->BindTexture(*m_pTexture);
	pShader->SetPosition(m_pPositionBuffer.get());
	pShader->SetTexture(m_pTexcoordBuffer.get());
	pShader->DrawElement(m_pPrimitive.GetType(), m_pIndexBuffer.get());
}

void RenderTextureNode::BuildGLBuffer()
{
	if (m_pPositionBuffer) { return; }
	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(m_pPrimitive.Position());

	m_pTexcoordBuffer = std::make_unique<GLBuffer>();
	m_pTexcoordBuffer->Create(m_pPrimitive.Texcoord());

	m_pIndexBuffer = std::make_unique<GLBuffer>();
	m_pIndexBuffer->Create(m_pPrimitive.Index());
	

}
void RenderTextureNode::UpdateRenderData()
{
}
}
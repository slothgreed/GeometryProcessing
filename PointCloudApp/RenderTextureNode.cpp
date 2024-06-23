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
	glEnable(GL_TEXTURE_2D);

	BuildGLBuffer();
	m_pShader->Use();
	OUTPUT_GLERROR;
	m_pShader->BindTexture(*m_pTexture);
	m_pShader->SetPosition(m_pPositionBuffer.get());
	m_pShader->SetTexture(m_pTexcoordBuffer.get());
	m_pShader->DrawElement(m_pPrimitive.GetType(), m_pIndexBuffer.get());
	OUTPUT_GLERROR;

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
	
	//m_pShader = dynamic_pointer_cast<TextureShader>(GetResource()->GetShader(IShadingShader::Type::Texture));
	m_pShader = std::make_shared<TextureShader>();
	m_pShader->Build();

}
void RenderTextureNode::UpdateRenderData()
{
}
}
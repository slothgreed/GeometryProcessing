#include "PostEffect.h"
#include "Primitives.h"
#include "SimpleShader.h"

namespace KI
{

void TextureDrawer::Execute(const DrawContext& context, const Texture* pTexture)
{
	auto pShader = context.pResource->GetShaderTable()->GetTextureViewShader();
	pShader->Use();
	pShader->BindTexture(*pTexture);
	pShader->Draw(*context.pResource->GetTexturePlane());
}

RenderTextureNode::RenderTextureNode()
{
	BuildGLBuffer();
}

void RenderTextureNode::BuildGLBuffer()
{
	RenderPlane plane;
	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(plane.Position());
	
	m_pTexcoordBuffer = std::make_unique<GLBuffer>();
	m_pTexcoordBuffer->Create(plane.Texcoord());
	
	m_pIndexBuffer = std::make_unique<GLBuffer>();
	m_pIndexBuffer->Create(plane.Index());
}
ShaderPath ComputeTextureCombiner::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "posteffect\\posteffect.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "posteffect\\combineComputeTexture.frag";
	return path;
}

void ComputeTextureCombiner::FetchUniformLocation()
{
	m_uColorForward = GetUniformLocation("u_colorForward");
	m_uDepthForward = GetUniformLocation("u_depthForward");
	m_uColorCompute = GetUniformLocation("u_colorCompute");
	m_uDepthCompute = GetUniformLocation("u_depthCompute");

}
void ComputeTextureCombiner::Execute(const DrawContext& context, RenderTarget& target)
{
	auto pTarget = context.pResource->GetRenderTarget();
	target.Copy(*pTarget);
	Use();
	BindTexture(m_uColorForward, 0, *target.GetColor(0));
	BindTexture(m_uDepthForward, 1, *target.GetDepth());
	BindTexture(m_uColorCompute, 2, *context.pResource->GetComputeColorTarget());
	BindTexture(m_uDepthCompute, 3, *context.pResource->GetComputeDepthTarget());
	Draw(*context.pResource->GetTexturePlane());
}


}
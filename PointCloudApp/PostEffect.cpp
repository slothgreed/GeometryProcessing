#include "PostEffect.h"
#include "Primitives.h"
#include "SimpleShader.h"

namespace KI
{

void TextureDrawer::Execute(const DrawContext& context, const Texture* pTexture)
{
	auto& pShader = context.pResource->GetShaderTable()->GetTextureViewShader();
	pShader->Use();
	pShader->BindTexture(*pTexture);
	pShader->Draw(*context.pResource->GetTexturePlane());
}

void TextureDrawer::Execute(const DrawContext& context, const CubemapTexture* pTexture, int mipmap)
{
	auto pShader = context.pResource->GetShaderTable()->GetCubemapViewShader();
	pShader->Use();
	pShader->BindMipmapLevel(mipmap);
	pShader->BindTexture(*pTexture);
	pShader->Draw(*context.pResource->GetTexturePlane());
}
TexturePlane::TexturePlane()
{
	BuildGLBuffer();
}

void TexturePlane::BuildGLBuffer()
{
	Vector<Vector3> position(4);
	position[0] = Vector3(-1.0, -1.0, 0.0);
	position[1] = Vector3(1.0, -1.0, 0.0);
	position[2] = Vector3(1.0, 1.0, 0.0);
	position[3] = Vector3(-1.0, 1.0, 0.0);

	Vector<Vector2> texcoord(4);
	texcoord[0] = Vector2(0, 0.0);
	texcoord[1] = Vector2(1.0, 0.0);
	texcoord[2] = Vector2(1.0, 1.0);
	texcoord[3] = Vector2(0, 1.0);

	Vector<UInt> index(6);
	index[0] = 0;	index[1] = 1;	index[2] = 2;
	index[3] = 0;	index[4] = 2;	index[5] = 3;

	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(position);

	m_pTexcoordBuffer = std::make_unique<GLBuffer>();
	m_pTexcoordBuffer->Create(texcoord);

	m_pIndexBuffer = std::make_unique<GLBuffer>();
	m_pIndexBuffer->Create(index);
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
	m_uImageSize = GetUniformLocation("u_ImageSize");

}
void ComputeTextureCombiner::Execute(const DrawContext& context)
{
	auto pTarget = context.pResource->GetRenderTarget();
	auto pTmp = context.pResource->GetTmpComputeTarget();
	pTmp->Copy(*pTarget);
	Use();
	BindUniform(m_uImageSize, context.pResource->GL()->GetWindowSize());
	BindTexture(m_uColorForward, 0, *pTmp->GetColor(0));
	BindTexture(m_uDepthForward, 1, *pTmp->GetDepth());
	BindShaderStorage(2, context.pResource->GetComputeColorTarget()->Handle());
	BindShaderStorage(3, context.pResource->GetComputeDepthTarget()->Handle());
	Draw(*context.pResource->GetTexturePlane());
}

ShaderPath EdgeDetector::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "posteffect\\posteffect.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "posteffect\\edgedetector.frag";
	return path;
}

void EdgeDetector::FetchUniformLocation()
{
	m_uColorTexture = GetUniformLocation("u_colorTex");
	m_uNormalTexture = GetUniformLocation("u_normalTex");
	m_uDepthTexture = GetUniformLocation("u_depthTex");
	m_uTexelSize = GetUniformLocation("u_texelSize");
	m_uDiffDepth = GetUniformLocation("u_diffDepth");
	m_uDiffNormal = GetUniformLocation("u_diffNormal");
}
void EdgeDetector::Execute(const DrawContext& context)
{
	Use();
	auto pTarget = context.pResource->GetRenderTarget();
	BindTexture(m_uColorTexture, 0, *pTarget->GetColor(0) );
	BindTexture(m_uNormalTexture, 1, *pTarget->GetNormal());
	BindTexture(m_uDepthTexture, 2, *pTarget->GetDepth());
	BindUniform(m_uDiffNormal, m_diffNormal);
	BindUniform(m_uDiffDepth, m_diffDepth);
	auto windowSize = context.pResource->GL()->GetWindowSize();
	
	BindUniform(m_uTexelSize, Vector2(1.0f / windowSize.x, 1.0f / windowSize.y));
	Draw(*context.pResource->GetTexturePlane());
}


void PostEffect::Execute(DrawContext& context)
{
	if (m_ui.edgeDetector) {
		if (!m_EdgeDetector.IsActive()) { m_EdgeDetector.Build(); }
		m_EdgeDetector.Execute(context);
	}

}
void PostEffect::ShowUI(UIContext& ui)
{
	ImVec2 window_size = ImVec2(300, 100);
	ImVec2 window_pos = ImVec2(
		ui.GetViewport().Size.x - window_size.x - 10.0f,
		ui.GetViewport().Size.y - window_size.y - 10.0f);

	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(window_size);
	ImGui::Begin("PostEffect");
	ImGui::Checkbox("EdgeDetector", &m_ui.edgeDetector);
	if (m_ui.edgeDetector) {
		ImGui::DragFloat("DepthDiff", &m_ui.diffDepth, 0.0f, 0.001f, 0.1f, "%.3f");
		ImGui::DragFloat("NormalDiff", &m_ui.diffNormal, 0.0f, 0.001f, 1.0f, "%.3f");
		m_EdgeDetector.SetDiffDepth(m_ui.diffDepth);
		m_EdgeDetector.SetDiffNormal(m_ui.diffNormal);
	}
	ImGui::End();
}

}
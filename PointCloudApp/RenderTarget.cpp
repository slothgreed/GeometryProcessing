#include "RenderTarget.h"
#include "Texture.h"
#include "Utility.h"
namespace KI
{
RenderTarget::RenderTarget()
	:m_pDepth(nullptr)
{
}

RenderTarget::~RenderTarget()
{
}

void RenderTarget::Bind(int drawTarget)
{
	m_pFrameBuffer->Bind();
	if (drawTarget == -1) {
		glDrawBuffers(m_attachment.size(), m_attachment.data());
	} else {
		glDrawBuffers(drawTarget, m_attachment.data());
	}
}

void RenderTarget::UnBind()
{
	m_pFrameBuffer->UnBind();
	glDrawBuffer(GL_BACK);
}

void RenderTarget::Build(Shared<Texture2D>&& pColor)
{
	m_pColor.push_back(std::move(pColor));
	Build();
}
void RenderTarget::Build(Shared<Texture2D>&& pColor, Shared<Texture2D>&& pDepth)
{
	m_pColor.push_back(std::move(pColor));
	m_pDepth = pDepth;

	Build();
}
void RenderTarget::Build(std::vector<Shared<Texture2D>>&& pColor, Shared<Texture2D>&& pDepth)
{
	m_pColor = std::move(pColor);
	m_pDepth = std::move(pDepth);

	Build();
}

void RenderTarget::Build()
{
	m_attachment.clear();
	m_pFrameBuffer = nullptr;

	m_pFrameBuffer = std::make_unique<FrameBuffer>();
	m_pFrameBuffer->Build();
	m_pFrameBuffer->Bind();
	for (int i = 0; i < m_pColor.size(); i++) {
		m_pColor[i]->Bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_pColor[i]->Handle(), 0);
		OUTPUT_GLERROR;
		m_attachment.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	if (m_pDepth) {
		m_pDepth->Bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_pDepth->Handle(), 0);
		OUTPUT_GLERROR;
	}

	m_pFrameBuffer->Validate();
	m_pFrameBuffer->UnBind();
}

void RenderTarget::Resize(const Vector2& size)
{
	for (auto& pColor : m_pColor) {
		pColor->Resize(size.x, size.y);
	}

	if (m_pDepth) {
		m_pDepth->Resize(size.x, size.y);
	}
}

RenderTarget* RenderTarget::CreatePickTarget(const Vector2i& size)
{
	auto pTarget = new RenderTarget();
	auto pColor = std::shared_ptr<Texture2D>(CreateColorTexture(size));
	auto pDepth = std::shared_ptr<Texture2D>(CreateDepthTexture(size));
	pTarget->Build(std::move(pColor), std::move(pDepth));
	return pTarget;
}

RenderTarget* RenderTarget::CreateForwardTarget(const Vector2i& size)
{
	auto pTarget = new RenderTarget();
	auto pColor = std::shared_ptr<Texture2D>(CreateColorTexture(size));
	auto pPosition = std::shared_ptr<Texture2D>(CreateColorFloatTexture(size));
	auto pNormal = std::shared_ptr<Texture2D>(CreateColorTexture(size));
	std::vector<Shared<Texture2D>> color;
	color.push_back(pColor);
	color.push_back(pPosition);
	color.push_back(pNormal);
	auto pDepth = std::shared_ptr<Texture2D>(CreateDepthTexture(size));
	pTarget->Build(std::move(color), std::move(pDepth));
	return pTarget;
}

RenderTarget* RenderTarget::CreatePostEffectTarget(const Vector2i& size)
{
	auto pTarget = new RenderTarget();
	auto pColor = std::shared_ptr<Texture2D>(CreateColorTexture(size));
	pTarget->Build(std::move(pColor));
	return pTarget;
}

Texture2D* RenderTarget::CreateColorFloatTexture(const Vector2i& size)
{
	auto pTexture = new Texture2D();
	pTexture->Build(size.x, size.y);
	Texture::Format format;
	format.target = GL_TEXTURE_2D;
	format.level = 0;
	format.internalformat = GL_RGB32F;
	format.width = size.x;
	format.height = size.y;
	format.border = 0;
	format.format = GL_RGBA;
	format.type = GL_FLOAT;
	pTexture->Set(format, nullptr);
	return pTexture;
}
Texture2D* RenderTarget::CreateColorTexture(const Vector2i& size)
{
	auto pTexture = new Texture2D();
	pTexture->Build(size.x, size.y);
	return pTexture;
}

Texture2D* RenderTarget::CreatePickTexture(const Vector2i& size)
{
	auto pTexture = new Texture2D();
	pTexture->Build(size.x, size.y);
	Texture::Format format;
	format.target = GL_TEXTURE_2D;
	format.level = 0;
	format.internalformat = GL_R32UI;
	format.width = size.x;
	format.height = size.y;
	format.border = 0;
	format.format = GL_RED_INTEGER;
	format.type = GL_UNSIGNED_INT;
	pTexture->Set(format, nullptr);

	return pTexture;
}


Texture2D* RenderTarget::CreateDepthTexture(const Vector2i& size)
{
	auto pDepth = new Texture2D();
	pDepth->Build(size.x, size.y);
	Texture::Format format;
	format.target = GL_TEXTURE_2D;
	format.level = 0;
	format.internalformat = GL_DEPTH_COMPONENT24;
	format.width = size.x;
	format.height = size.y;
	format.border = 0;
	format.format = GL_DEPTH_COMPONENT;
	format.type = GL_FLOAT;
	pDepth->Set(format, nullptr);
	return pDepth;
}
int RenderTarget::GetIntPixel(int x, int y)
{
	if (m_pColor.size() == 0) { return -1; }

	if (m_pColor[0]->GetFormat().format == GL_RED_INTEGER) {
		m_pFrameBuffer->Bind();
		GLuint result;
		glReadPixels(x, y, 1, 1, m_pColor[0]->GetFormat().format, m_pColor[0]->GetFormat().type, &result);
		m_pFrameBuffer->UnBind();
		return result;
	} else {
		m_pFrameBuffer->Bind();
		unsigned char result[4];
		glReadPixels(x, y, 1, 1, m_pColor[0]->GetFormat().format, m_pColor[0]->GetFormat().type, &result);
		uint r = uint(result[0]); 
		uint g = uint(result[1]);
		uint b = uint(result[2]); 
		uint a = uint(result[3]); 
		uint decodedPrimitiveID = (r << 24) | (g << 16) | (b << 8) | a;
		//printf(glmUtil::ToString(result).data()); printf("\n");

		m_pFrameBuffer->UnBind();
		return decodedPrimitiveID;
	}
	
}

float RenderTarget::GetDepth(int x, int y)
{
	m_pFrameBuffer->Bind();
	float result;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &result);
	OUTPUT_GLERROR;
	m_pFrameBuffer->UnBind();
	return result;
}

void RenderTarget::Copy(const RenderTarget& target)
{
	if (GetColorNum() != target.GetColorNum()) { return; }
	if (GetDepthNum() != target.GetDepthNum()) { return; }
	for (int i = 0; i < m_pColor.size(); i++) {
		m_pColor[i]->Copy(*target.GetColor(i).get());
	}

	if (m_pDepth) {
		m_pDepth->Copy(*target.GetDepth().get());
	}
}
}

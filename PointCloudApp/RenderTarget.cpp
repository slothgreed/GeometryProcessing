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

void RenderTarget::Bind()
{
	m_pFrameBuffer->Bind();
	glDrawBuffers(m_attachment.size(), m_attachment.data());
}

void RenderTarget::UnBind()
{
	m_pFrameBuffer->UnBind();
	glDrawBuffer(GL_BACK);
}
void RenderTarget::Build(const Shared<Texture2D>& pColor, const Shared<Texture2D>& pDepth)
{
	m_attachment.clear();
	m_pFrameBuffer = nullptr;

	m_pFrameBuffer = std::make_unique<FrameBuffer>();
	m_pFrameBuffer->Build();
	m_pFrameBuffer->Bind();
	m_pColor.push_back(std::move(pColor));
	m_pDepth = std::move(pDepth);
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

RenderTarget* RenderTarget::CreatePickTarget(const Vector2& size)
{
	auto pTarget = new RenderTarget();
	auto pColor = std::shared_ptr<Texture2D>(CreateColorTexture(size.x, size.y));
	auto pDepth = std::shared_ptr<Texture2D>(CreateDepthTexture(size.x, size.y));
	pTarget->Build(pColor, pDepth);
	return pTarget;
}

Texture2D* RenderTarget::CreateColorTexture(int width, int height)
{
	auto pTexture = new Texture2D();
	pTexture->Build(width, height);
	return pTexture;
}

Texture2D* RenderTarget::CreatePickTexture(int width, int height)
{
	auto pTexture = new Texture2D();
	pTexture->Build(width, height);
	Texture::Format format;
	format.target = GL_TEXTURE_2D;
	format.level = 0;
	format.internalformat = GL_R32UI;
	format.width = width;
	format.height = height;
	format.border = 0;
	format.format = GL_RED_INTEGER;
	format.type = GL_UNSIGNED_INT;
	pTexture->Set(format);

	return pTexture;
}


Texture2D* RenderTarget::CreateDepthTexture(int width, int height)
{
	auto pDepth = new Texture2D();
	pDepth->Build(width, height);
	Texture::Format format;
	format.target = GL_TEXTURE_2D;
	format.level = 0;
	format.internalformat = GL_DEPTH24_STENCIL8;
	format.width = width;
	format.height = height;
	format.border = 0;
	format.format = GL_DEPTH_STENCIL;
	format.type = GL_UNSIGNED_INT_24_8;
	pDepth->Set(format);
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

}

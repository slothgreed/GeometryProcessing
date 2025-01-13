#include "SkyBoxNode.h"
#include "Primitives.h"
#include "Utility.h"
namespace KI
{

SkyBoxNode::Shader::Shader()
{
}
SkyBoxNode::Shader::~Shader()
{
}

ShaderPath SkyBoxNode::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "skybox.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "skybox.frag";
	return path;
}

void SkyBoxNode::Shader::FetchUniformLocation()
{
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
	m_uniform[UNIFORM::SKYBOX] = glGetUniformLocation(Handle(), "u_skybox");
}

void SkyBoxNode::Shader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
}

void SkyBoxNode::Shader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}
void SkyBoxNode::Shader::SetPosition(GLBuffer* pPosition)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pPosition));
	glBindVertexBuffer(ATTRIB_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
}

void SkyBoxNode::Shader::SetTexture(Texture* pTexture)
{
	glBindTextureUnit(0, pTexture->Handle());
	glUniform1i(m_uniform[UNIFORM::SKYBOX], 0);

}
void SkyBoxNode::Shader::SetTexcoord(GLBuffer* pTexcoord)
{
	SetVertexFormat(VertexFormat(ATTRIB_TEXCOORD, pTexcoord));
	glBindVertexBuffer(ATTRIB_TEXCOORD, pTexcoord->Handle(), 0, pTexcoord->SizeOfData());
}

SkyBoxNode::SkyBoxNode()
	: RenderNode("SkyBox")
	, m_pShader(nullptr)
	, m_pCubemap(nullptr)
{
}

SkyBoxNode::~SkyBoxNode()
{
}

Vector<String> SkyBoxNode::GetTexturePath()
{
	String directory = "E:\\cgModel\\skybox\\";
	Vector<String> path(6);
	path[0] = (directory + "right.jpg");
	path[1] = (directory + "left.jpg");
	path[2] = (directory + "top.jpg");
	path[3] = (directory + "bottom.jpg");
	path[4] = (directory + "front.jpg");
	path[5] = (directory + "back.jpg");

	return path;
}

void SkyBoxNode::BuildGLBuffer()
{
	if (m_skybox != nullptr) { return; }
	m_skybox = std::make_unique<SkyBox>();
	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(m_skybox->Position());

	m_pShader = std::make_unique<SkyBoxNode::Shader>();
	m_pShader->Build();

	m_pCubemap = std::make_unique<CubemapTexture>();
	m_pCubemap->Build(GetTexturePath());
}
void SkyBoxNode::Draw(const DrawContext& context)
{
	BuildGLBuffer();

	m_pShader->Use();
	m_pShader->SetCamera(context.pResource->GetCameraBuffer());
	m_pShader->SetPosition(m_pPositionBuffer.get());
	m_pShader->SetTexture(m_pCubemap.get());
	m_pShader->SetModel(glmUtil::CreateScale(100));
	m_pShader->DrawArray(m_skybox->GetDrawType(), m_pPositionBuffer->Num());
}

}

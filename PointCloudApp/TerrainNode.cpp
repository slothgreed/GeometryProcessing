#include "TerrainNode.h"
#include "SimpleShader.h"
#include "TextureLoader.h"
#include "Utility.h"
namespace KI
{

TerrainNode::TerrainNode()
	:RenderNode("Terrain")
	,m_pPositionBuffer(nullptr)
	,m_pTexcoordBuffer(nullptr)
	,m_pRGB(nullptr)
	,m_pHeight(nullptr)
{
}

TerrainNode::~TerrainNode()
{
}

TerrainNode::Shader::Shader()
	: m_ulevelInner(-1)
	, m_ulevelOuter(-1)
	, m_uModel(-1)
	, m_uModelScale(-1)
	, m_utexture(-1)
	, m_uHeight(-1)
	, m_uheightMap(-1)
	, m_ushowWire(-1)
{

}
ShaderPath TerrainNode::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "terrain\\terrain.vert";
	path.shader[SHADER_PROGRAM_TES_CONTROL] = "terrain\\terrain.tcs";
	path.shader[SHADER_PROGRAM_TES_EVAL] = "terrain\\terrain.tes";
	path.shader[SHADER_PROGRAM_FRAG] = "terrain\\terrain.frag";
	return path;
}

void TerrainNode::Shader::FetchUniformLocation()
{
	m_uModel = GetUniformLocation("u_Model");
	m_uModelScale = GetUniformLocation("u_ModelScale");
	m_ulevelOuter = GetUniformLocation("u_levelOuter");
	m_ulevelInner = GetUniformLocation("u_levelInner");
	m_utexture = GetUniformLocation("u_texture");
	m_uheightMap = GetUniformLocation("u_heightMap");
	m_ushowWire = GetUniformLocation("u_showWire");
	m_uHeight = GetUniformLocation("u_Height");
}

void TerrainNode::Shader::SetPosition(GLBuffer* pPosition)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pPosition));
	glBindVertexBuffer(ATTRIB_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
}
void TerrainNode::Shader::SetTexcoord(GLBuffer* pTexcoord)
{
	SetVertexFormat(VertexFormat(ATTRIB_TEXCOORD, pTexcoord));
	glBindVertexBuffer(ATTRIB_TEXCOORD, pTexcoord->Handle(), 0, pTexcoord->SizeOfData());
}

void TerrainNode::Shader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
	OUTPUT_GLERROR;
}

void TerrainNode::Shader::SetMatrix(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pBuffer->Handle());
	OUTPUT_GLERROR;
}


void TerrainNode::Shader::BindShowWire(bool wire)
{
	BindUniform(m_ushowWire, wire ? 1 : 0);
}
void TerrainNode::Shader::BindTessLevel(int inner, int outer)
{
	BindUniform(m_ulevelInner, inner);
	BindUniform(m_ulevelOuter, outer);
}
void TerrainNode::Shader::BindMatrix(const Matrix4x4& matrix, float scale)
{
	BindUniform(m_uModel, matrix);
	BindUniform(m_uModelScale, scale);
}
void TerrainNode::Shader::BindHeight(float height)
{
	BindUniform(m_uHeight, height);
}
void TerrainNode::Shader::BindTexture(const Texture& rgb, const Texture& height)
{
	IShadingShader::BindTexture(m_utexture, 0, rgb);
	IShadingShader::BindTexture(m_uheightMap, 1, height);
}

Vector<TerrainNode::InstanceData> CreateInstance(int num, float scale)
{
	Vector<TerrainNode::InstanceData> instance;
	instance.reserve(num * num);
	auto half = num / 2;
	for (int i = -half; i < half; i++) {
		for (int j = -half; j < half; j++) {
			auto data = TerrainNode::InstanceData();
			data.matrix = glmUtil::CreateTransform(scale, Vector3(i * scale, 0, j * scale));
			data.scale = scale;
			instance.push_back(std::move(data));
		}
	}

	return instance;
}
TerrainNode::Patch CreatePatch(const Vector3& min, const Vector3& max, int div)
{
	TerrainNode::Patch patch;

	const float dx = (max.x - min.x) / div;
	const float dz = (max.z - min.z) / div;

	const float du = 1.0f / div;
	const float dv = 1.0f / div;

	patch.position.reserve(div * div * 4);
	patch.texcoord.reserve(div * div * 4);

	for (int z = 0; z < div; ++z) {
		for (int x = 0; x < div; ++x) {
			// ---- world position ----
			float x0 = min.x + dx * x;
			float x1 = min.x + dx * (x + 1);
			float z0 = min.z + dz * z;
			float z1 = min.z + dz * (z + 1);

			// ---- texcoord ----
			float u0 = du * x;
			float u1 = du * (x + 1);
			float v0 = dv * z;
			float v1 = dv * (z + 1);

			patch.position.push_back({ x0, min.y, z0 });
			patch.position.push_back({ x0, min.y, z1 });
			patch.position.push_back({ x1, min.y, z0 });
			patch.position.push_back({ x1, min.y, z1 });

			patch.texcoord.push_back({ u0, v0, 0.0f });
			patch.texcoord.push_back({ u0, v1, 0.0f });
			patch.texcoord.push_back({ u1, v0, 0.0f });
			patch.texcoord.push_back({ u1, v1, 0.0f });
		}
	}

	return patch;
}
void TerrainNode::BuildResource()
{
	if (!m_pShader) {
		m_pShader = std::make_unique<TerrainNode::Shader>();
		m_pShader->Build();
	}

	if (m_pPositionBuffer) { return; }
	
	m_patch = CreatePatch(Vector3(0), Vector3(1), 8);
	m_pPositionBuffer = std::make_unique<GLBuffer>();
	m_pPositionBuffer->Create(m_patch.position);
	m_pTexcoordBuffer = std::make_unique<GLBuffer>();
	m_pTexcoordBuffer->Create(m_patch.texcoord);
	auto instance = CreateInstance(100, GetScale());
	m_pInstanceBuffer = std::make_unique<GLBuffer>();
	m_pInstanceBuffer->Create(instance);

	m_pRGB = std::unique_ptr<Texture>(TextureLoader::Load("E:\\cgModel\\terrain\\ganges_river_pebbles_diff_512.png"));
	m_pHeight = std::unique_ptr<Texture>(TextureLoader::Load("E:\\cgModel\\terrain\\ganges_river_pebbles_disp_512.png"));
}
void TerrainNode::Draw(const DrawContext& context)
{
	if (!m_ui.visible) { return; }
	BuildResource();
	m_pShader->Use();
	m_pShader->PatchParameteri(4);
	m_pShader->BindShowWire(false);
	m_pShader->BindTexture(*m_pRGB ,*m_pHeight);
	m_pShader->BindTessLevel(1 << m_ui.tessInner, 1 << m_ui.tessOuter);
	m_pShader->BindMatrix(GetMatrix(), GetScale());
	m_pShader->BindHeight(m_ui.height);

	m_pShader->SetPosition(m_pPositionBuffer.get());
	m_pShader->SetTexcoord(m_pTexcoordBuffer.get());
	m_pShader->SetCamera(context.pResource->GetCameraBuffer());
	m_pShader->SetMatrix(m_pInstanceBuffer.get());
	m_pShader->DrawArrayInstaced(GL_PATCHES, m_patch.position.size(),m_pInstanceBuffer->Num());
	if (m_ui.visibleWire) {
		context.pResource->GL()->EnablePolygonWire();
		m_pShader->BindShowWire(true);
		m_pShader->DrawArrayInstaced(GL_PATCHES, m_patch.position.size(),m_pInstanceBuffer->Num());
		context.pResource->GL()->EnablePolygonFill();
	}
}


void TerrainNode::ShowUI(UIContext& context)
{
	ImGui::Checkbox("TerrainVisible", &m_ui.visible);
	ImGui::Checkbox("TerrainVisibleWire", &m_ui.visibleWire);
	ImGui::SliderInt("TerrainTessInner", &m_ui.tessInner, 0, 6);
	ImGui::SliderInt("TerrainTessOuter", &m_ui.tessOuter, 0, 6);
	ImGui::SliderInt("TerrainHeight", &m_ui.height, 1, GetScale());
}





}
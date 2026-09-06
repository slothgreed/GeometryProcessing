#include "TileLightCuller.h"
#include "RenderResource.h"
#include "RenderNode.h"
#include "Camera.h"
namespace KI
{
void TileLightCuller::DebugViewShader::FetchUniformLocation()
{
	m_uTileCount = GetUniformLocation("u_tileCount");
	m_uTileSize = GetUniformLocation("u_tileSize");
	m_uMaxLightNum = GetUniformLocation("u_maxLightNum");
}

ShaderPath TileLightCuller::DebugViewShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "posteffect\\posteffect.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "algorithm\\tileLighViewer.frag";
	return path;
}

void TileLightCuller::DebugViewShader::BindTileCount(const Vector2i& tileCount)
{
	BindUniform(m_uTileCount, tileCount);
}

void TileLightCuller::DebugViewShader::BindTileSize(const Vector2i& tileSize)
{
	BindUniform(m_uTileSize, tileSize);
}
void TileLightCuller::DebugViewShader::BindMaxLightNum()
{
	BindUniform(m_uMaxLightNum, MAX_LIGHT_NUM);
}


void TileLightCuller::Shader::FetchUniformLocation()
{
	m_uDepth = GetUniformLocation("u_depthTexture");
	m_uWindowSize = GetUniformLocation("u_windowSize");
	m_uLocalSize = GetUniformLocation("u_localSize");
	m_uLightNum = GetUniformLocation("u_lightNum");
}

void TileLightCuller::Shader::BindDepth(const Texture* pTexture)
{
	BindTexture(m_uDepth, 0, pTexture);
}
void TileLightCuller::Shader::BindWindowSize(const Vector2i& windowSize)
{
	BindUniform(m_uWindowSize, windowSize);
}
void TileLightCuller::Shader::BindLocalSize(const Vector2i& localSize)
{
	BindUniform(m_uLocalSize, localSize);
}
void TileLightCuller::Shader::BindLightNum(int lightNum)
{
	BindUniform(m_uLightNum, lightNum);
}
ShaderPath TileLightCuller::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_COMPUTE] = "algorithm\\tileLightCuller.comp";
	return path;
}

void TileLightCuller::UpdateShader::FetchUniformLocation()
{
	m_uBDBMin = GetUniformLocation("u_bdbMin");
	m_uBDBMax = GetUniformLocation("u_bdbMax");
	m_uTimeDelta = GetUniformLocation("u_timeDelta");
}
ShaderPath TileLightCuller::UpdateShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_COMPUTE] = "algorithm\\tileLightUpdate.comp";
	return path;
}
void TileLightCuller::UpdateShader::BindBoundingBox(const Vector3& min, const Vector3& max)
{
	BindUniform(m_uBDBMin, min);
	BindUniform(m_uBDBMax, max);
}
void TileLightCuller::UpdateShader::BindTimeDelta(float timeDelta)
{
	BindUniform(m_uTimeDelta, timeDelta);
}
void TileLightCuller::DrawDebugView(const DrawContext& context)
{
	if (m_pDebugShader == nullptr) {
		m_pDebugShader = new DebugViewShader();
		m_pDebugShader->Build();
	}
	context.pResource->GL()->EnableBlend();
	m_pDebugShader->Use();
	m_pDebugShader->BindTileCount(GetTileCount2D(context.pResource->GetCamera()->ViewSize()));
	m_pDebugShader->BindShaderStorage(1, context.pResource->GetTileLightBuffer()->Handle());
	m_pDebugShader->BindTileSize(GetTileSize());
	m_pDebugShader->BindMaxLightNum();
	m_pDebugShader->Draw(*context.pResource->GetTexturePlane());
	context.pResource->GL()->DisableBlend();
}

void TileLightCuller::Update(const DrawContext& context, const BDB& bdb)
{
	if (m_pUpdateShader == nullptr) {
		m_pUpdateShader = new UpdateShader();
		m_pUpdateShader->Build();
	}
	BuildResource(context.pResource->GetCamera()->ViewSize());
	m_pUpdateShader->Use();
	m_pUpdateShader->BindShaderStorage(2, context.pResource->GetPointLightBuffer()->Handle());
	m_pUpdateShader->BindBoundingBox(bdb.Min(), bdb.Max());
	m_pUpdateShader->BindTimeDelta(context.pResource->GetTimeDelta() * 10);
	m_pUpdateShader->Dispatch1D(context.pResource->GetPointLightBuffer()->Num());
	m_pUpdateShader->BarrierSSBO();

}
void TileLightCuller::Execute(const DrawContext& context)
{
	auto viewSize = context.pResource->GetCamera()->ViewSize();
	BuildResource(context.pResource->GetCamera()->ViewSize());
	m_pShader->Use();
	m_pShader->BindDepth(context.pResource->GetRenderTarget()->GetDepth().get());
	m_pShader->BindWindowSize(viewSize);
	m_pShader->BindLocalSize(m_pShader->GetLocalThread2D());
	m_pShader->BindLightNum(context.pResource->GetPointLightBuffer()->Num());

	m_pShader->BindShaderStorage(0, context.pResource->GetCameraBuffer()->Handle());
	m_pShader->BindShaderStorage(1, context.pResource->GetTileLightBuffer()->Handle());
	m_pShader->BindShaderStorage(2, context.pResource->GetPointLightBuffer()->Handle());
	m_pShader->Dispatch(m_pShader->GetDispatchNum2D(viewSize));
	m_pShader->BarrierSSBO();
}

void TileLightCuller::BuildResource(const Vector2i& windowSize)
{
	if (m_pShader == nullptr) {
		m_pShader = new TileLightCuller::Shader();
		m_pShader->Build();
	}
}

Vector2i TileLightCuller::GetTileCount2D(const Vector2i& windowSize)
{
	return
		Vector2i(
			MathHelper::CeilDiv(windowSize.x, PIXEL_SIZE),
			MathHelper::CeilDiv(windowSize.y, PIXEL_SIZE));
}
int TileLightCuller::GetTileCount1D(const Vector2i& windowSize)
{
	auto count = GetTileCount2D(windowSize);
	return count.x * count.y;
}

void TileLightResource::BuildPointLightBuffer(const Vector2i& windowSize)
{
	if (m_pTileLight == nullptr) {
		m_pTileLight = new GLBuffer();
	}

	struct TileLight
	{
		int count;
		float minDepth; float maxDepth; int pad2;
		int tileLight[8];
	};

	auto tileCount = TileLightCuller::GetTileCount1D(windowSize);
	if (m_pTileLight->Num() == tileCount) { return; }
	m_pTileLight->Create(tileCount, sizeof(TileLight));

}


void TileLightResource::BuildPointLights(const BDB& bdb, int resolution)
{
	BuildPointLights(bdb, Vector3i(resolution));
}

void TileLightResource::BuildPointLights(const BDB& bdb, const Vector3i& resolution)
{
	m_bdb = bdb;
	if (!bdb.IsActive()) {
		throw std::invalid_argument("Point light BDB must be active.");
	}
	if (resolution.x <= 0 || resolution.y <= 0 || resolution.z <= 0) {
		throw std::invalid_argument("Point light resolution must be positive.");
	}

	const size_t maxLightCount = static_cast<size_t>(std::numeric_limits<int>::max());
	const size_t resolutionX = static_cast<size_t>(resolution.x);
	const size_t resolutionY = static_cast<size_t>(resolution.y);
	const size_t resolutionZ = static_cast<size_t>(resolution.z);
	if (resolutionX > maxLightCount / resolutionY ||
		resolutionX * resolutionY > maxLightCount / resolutionZ) {
		throw std::overflow_error("Point light count exceeds GLBuffer capacity.");
	}
	const size_t lightCount = resolutionX * resolutionY * resolutionZ;

	const Vector3 pitch = (bdb.Max() - bdb.Min()) / Vector3(resolution);
	// A light is placed at the center of each grid cell.  The farthest point in
	// the cell is half of its diagonal, so use a slightly larger radius to avoid
	// unlit gaps between neighboring light volumes.
	const float radius = glm::length(pitch) * 0.6f;
	Vector<ShaderLayout::PointLight> pointLights;
	pointLights.reserve(lightCount);

	for (int z = 0; z < resolution.z; ++z) {
		for (int y = 0; y < resolution.y; ++y) {
			for (int x = 0; x < resolution.x; ++x) {
				const Vector3 gridPosition = Vector3(x, y, z) + Vector3(0.5f);
				ShaderLayout::PointLight pointLight;
				pointLight.positionRadius = Vector4(bdb.Min() + gridPosition * pitch, radius);
				pointLight.colorIntensity = Vector4(Random::Vec3(0, 1), 1.0f);
				pointLight.velocity = Vector4(Random::Vec3(-1, 1), 1.0f);
				pointLights.push_back(pointLight);
			}
		}
	}

	if (!m_pPointLightGpu) {
		m_pPointLightGpu = new GLBuffer();
	}
	m_pPointLightGpu->Create(pointLights);
}

}

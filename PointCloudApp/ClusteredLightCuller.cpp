#include "ClusteredLightCuller.h"
#include "RenderResource.h"
#include "RenderNode.h"
#include "Camera.h"
namespace KI
{
void ClusteredLightCuller::ShowUI(ClusteredLightResource& resource, const BDB& bdb, const Vector2i& windowSize)
{
	const char* lightCounts[] = { "1", "2", "4", "8", "16", "32"};
	// One cell places a single light at the bounding box center.
	const Vector3i resolutions[] = {
		Vector3i(1), Vector3i(2), Vector3i(4), Vector3i(8),
		Vector3i(16), Vector3i(32)
	};
	if (ImGui::Checkbox("ClusteredLight", &m_ui.enabled)) {
		auto clusteredLightResource = &resource;
		if(!clusteredLightResource->IsActive()) {
			clusteredLightResource->BuildPointLights(bdb, resolutions[m_ui.pointLightCountIndex]);
		}
	}
	if (m_ui.enabled) {
		auto clusteredLightResource = &resource;
		if (ImGui::Combo("Point Light Count", &m_ui.pointLightCountIndex, lightCounts, 6)) {
			clusteredLightResource->BuildPointLights(bdb, resolutions[m_ui.pointLightCountIndex]);
		}
		clusteredLightResource->BuildPointLightBuffer(windowSize);
		const char* modes[] = { "Hidden", "Minimum Depth", "Maximum Depth", "Light Count" };
		ImGui::Combo("Clustered Light View", &m_ui.debugMode, modes, 4);
		if (m_ui.debugMode > 0) {
			ImGui::SliderInt("Z Slice", &m_ui.debugSlice, 0, GetClusterPartition().z - 1);
			ImGui::Checkbox("Show Cluster Grid", &m_ui.debugGrid);
			ImGui::TextUnformatted("Magenta: light list overflow" );
		}
		ImGui::Checkbox("AnimationClusteredLight", &m_ui.animation);
	}
}

void ClusteredLightCuller::DebugViewShader::FetchUniformLocation()
{
	m_uClusterCount = GetUniformLocation("u_clusterCount");
	m_uClusterPartition = GetUniformLocation("u_clusterPartition");
	m_uMaxLightNum = GetUniformLocation("u_maxLightNum");
	m_uDisplayMode = GetUniformLocation("u_displayMode");
	 m_uSlice = GetUniformLocation("u_slice");
	 m_uShowGrid = GetUniformLocation("u_showGrid");
}

ShaderPath ClusteredLightCuller::DebugViewShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "posteffect\\posteffect.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "algorithm\\clusteredLightViewer.frag";
	return path;
}

void ClusteredLightCuller::DebugViewShader::BindClusterCount(const Vector2i& clusterCount)
{
	BindUniform(m_uClusterCount, clusterCount);
}

void ClusteredLightCuller::DebugViewShader::BindClusterPartition(const Vector3i& clusterPartition)
{
	BindUniform(m_uClusterPartition, clusterPartition);
}
void ClusteredLightCuller::DebugViewShader::BindMaxLightNum()
{
	BindUniform(m_uMaxLightNum, MAX_LIGHT_NUM);
}



void ClusteredLightCuller::DebugViewShader::BindDisplayMode(int displayMode)
{
	BindUniform(m_uDisplayMode, displayMode);
}

void ClusteredLightCuller::DebugViewShader::BindSlice(int slice)
{
	BindUniform(m_uSlice, slice);
}

void ClusteredLightCuller::DebugViewShader::BindShowGrid(bool showGrid)
{
	BindUniform(m_uShowGrid, int(showGrid));
}

void ClusteredLightCuller::Shader::FetchUniformLocation()
{
	m_uDepth = GetUniformLocation("u_depthTexture");
	m_uWindowSize = GetUniformLocation("u_windowSize");
	m_uClusterPartition = GetUniformLocation("u_clusterPartition");
	m_uLightNum = GetUniformLocation("u_lightNum");
}

void ClusteredLightCuller::Shader::BindDepth(const Texture* pTexture)
{
	BindTexture(m_uDepth, 0, pTexture);
}
void ClusteredLightCuller::Shader::BindWindowSize(const Vector2i& windowSize)
{
	BindUniform(m_uWindowSize, windowSize);
}
void ClusteredLightCuller::Shader::BindClusterPartition(const Vector3i& clusterNum)
{
	BindUniform(m_uClusterPartition, clusterNum);
}
void ClusteredLightCuller::Shader::BindLightNum(int lightNum)
{
	BindUniform(m_uLightNum, lightNum);
}
ShaderPath ClusteredLightCuller::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_COMPUTE] = "algorithm\\clusteredLightCuller.comp";
	return path;
}

void ClusteredLightCuller::UpdateShader::FetchUniformLocation()
{
	m_uBDBMin = GetUniformLocation("u_bdbMin");
	m_uBDBMax = GetUniformLocation("u_bdbMax");
	m_uTimeDelta = GetUniformLocation("u_timeDelta");
}
ShaderPath ClusteredLightCuller::UpdateShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_COMPUTE] = "algorithm\\clusteredLightUpdate.comp";
	return path;
}
void ClusteredLightCuller::UpdateShader::BindBoundingBox(const Vector3& min, const Vector3& max)
{
	BindUniform(m_uBDBMin, min);
	BindUniform(m_uBDBMax, max);
}
void ClusteredLightCuller::UpdateShader::BindTimeDelta(float timeDelta)
{
	BindUniform(m_uTimeDelta, timeDelta);
}
void ClusteredLightCuller::DrawDebugView(const DrawContext& context, int displayMode)
{
	if (m_pDebugShader == nullptr) {
		m_pDebugShader = new DebugViewShader();
		m_pDebugShader->Build();
	}
	context.pResource->GL()->EnableBlend();
	m_pDebugShader->Use();
	m_pDebugShader->BindClusterCount(GetClusterCount3D(context.pResource->GetCamera()->ViewSize()));
	m_pDebugShader->BindShaderStorage(1, context.pResource->GetClusteredLightResource()->GetClusteredLightBuffer()->Handle());
	m_pDebugShader->BindClusterPartition(GetClusterPartition());
	m_pDebugShader->BindMaxLightNum();
	m_pDebugShader->BindDisplayMode(displayMode);
	m_pDebugShader->BindSlice(std::clamp(m_ui.debugSlice, 0, GetClusterPartition().z - 1));
	m_pDebugShader->BindShowGrid(m_ui.debugGrid);
	const auto& camera = *context.pResource->GetCamera();
	const auto& bdb = context.pResource->GetClusteredLightResource()->GetBoundingBox();
	m_pDebugShader->Draw(*context.pResource->GetTexturePlane());
	context.pResource->GL()->DisableBlend();
}

void ClusteredLightCuller::Update(const DrawContext& context, const BDB& bdb)
{
	if (m_pUpdateShader == nullptr) {
		m_pUpdateShader = new UpdateShader();
		m_pUpdateShader->Build();
	}
	BuildResource(context.pResource->GetCamera()->ViewSize());
	m_pUpdateShader->Use();
	m_pUpdateShader->BindShaderStorage(2, context.pResource->GetClusteredLightResource()->GetPointLightBuffer()->Handle());
	m_pUpdateShader->BindBoundingBox(bdb.Min(), bdb.Max());
	m_pUpdateShader->BindTimeDelta(context.pResource->GetTimeDelta());
	m_pUpdateShader->Dispatch1D(context.pResource->GetClusteredLightResource()->GetPointLightBuffer()->Num());
	m_pUpdateShader->BarrierSSBO();

}
void ClusteredLightCuller::Execute(const DrawContext& context)
{
	auto viewSize = context.pResource->GetCamera()->ViewSize();
	BuildResource(viewSize);
	m_pShader->Use();
	m_pShader->BindDepth(context.pResource->GetRenderTarget()->GetDepth().get());
	m_pShader->BindWindowSize(viewSize);
	m_pShader->BindClusterPartition(GetClusterPartition());
	m_pShader->BindLightNum(context.pResource->GetClusteredLightResource()->GetPointLightBuffer()->Num());

	m_pShader->BindShaderStorage(0, context.pResource->GetCameraBuffer()->Handle());
	m_pShader->BindShaderStorage(1, context.pResource->GetClusteredLightResource()->GetClusteredLightBuffer()->Handle());
	m_pShader->BindShaderStorage(2, context.pResource->GetClusteredLightResource()->GetPointLightBuffer()->Handle());
	m_pShader->Dispatch(m_pShader->GetDispatchNum3D(Vector3i(viewSize.x, viewSize.y, PIXEL_SIZE)));
	m_pShader->BarrierSSBO();
}

void ClusteredLightCuller::BuildResource(const Vector2i& windowSize)
{
	if (m_pShader == nullptr) {
		m_pShader = new ClusteredLightCuller::Shader();
		m_pShader->Build();
	}
}

Vector3i ClusteredLightCuller::GetClusterCount3D(const Vector2i& windowSize)
{
	return
		Vector3i(
			MathHelper::CeilDiv(windowSize.x, PIXEL_SIZE),
			MathHelper::CeilDiv(windowSize.y, PIXEL_SIZE), PIXEL_SIZE);
}
int ClusteredLightCuller::GetClusterCount1D(const Vector2i& windowSize)
{
	auto count = GetClusterCount3D(windowSize);
	return count.x * count.y * count.z;
}

void ClusteredLightResource::BuildPointLightBuffer(const Vector2i& windowSize)
{
	if (m_pClusteredLight == nullptr) {
		m_pClusteredLight = new GLBuffer();
	}

	struct ClusteredLight
	{
		int count;
		float minDepth; float maxDepth; float debug;
		int clusteredLight[ClusteredLightCuller::MAX_LIGHT_NUM];
	};

	auto clusterCount = ClusteredLightCuller::GetClusterCount1D(windowSize);
	if (m_pClusteredLight->Num() == clusterCount) {
		return;
	}
	m_pClusteredLight->Create(clusterCount, sizeof(ClusteredLight));

}


void ClusteredLightResource::BuildPointLights(const BDB& bdb, int resolution)
{
	BuildPointLights(bdb, Vector3i(resolution));
}

void ClusteredLightResource::BuildPointLights(const BDB& bdb, const Vector3i& resolution)
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
	const float radius = glm::length((bdb.Max() - bdb.Min())) * 0.6f / 25;
	Vector<ShaderLayout::PointLight> pointLights;
	pointLights.reserve(lightCount);
	if(resolution.x == 1 && resolution.y == 1 && resolution.z == 1) {
		ShaderLayout::PointLight pointLight;
		auto center = bdb.Center();
		center.y = bdb.Min().y + (bdb.Center().y - bdb.Min().y) * 0.25f;
		pointLight.positionRadius = Vector4(center, radius);
		pointLight.colorIntensity = Vector4(Random::Vec3(0, 1), 1.0f);
		pointLight.velocity = Vector4(Random::Vec3(-1, 1), 1.0f);
		pointLights.push_back(pointLight);
	} else {
		for (int z = 0; z < resolution.z; ++z) {
		for (int y = 0; y < resolution.y; ++y) {
		for (int x = 0; x < resolution.x; ++x) {
			const Vector3 gridPosition = Vector3(x, y, z) + Vector3(0.5f);
			ShaderLayout::PointLight pointLight;
			pointLight.positionRadius = Vector4(bdb.Min() + gridPosition * pitch, radius);
			pointLight.colorIntensity = Vector4(Random::Vec3(0, 1), 1.0f);
			pointLight.velocity = Vector4(Random::Vec3(-100, 100), 1.0f);
			pointLights.push_back(pointLight);
		}}}
	}
	if (!m_pPointLightGpu) {
		m_pPointLightGpu = new GLBuffer();
	}
	m_pPointLightGpu->Create(pointLights);
}

}

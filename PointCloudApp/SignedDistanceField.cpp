#include "SignedDistanceField.h"
#include "HalfEdgeNode.h"
#include "Primitives.h"
#include "PrimitiveNode.h"
#include "Texture.h"
#include "Utility.h"
//#define GPU_DEBUG
namespace KI
{
SignedDistanceField::SignedDistanceField(HalfEdgeNode* pHalfEdge)
	: m_pHalfEdge(pHalfEdge)
	, m_resolute(4)
	, m_frequency(2.0f)
{
	auto sampler = Texture::Sampler(Texture::Sampler::FILTER::NEAREST);
	m_gpu.xTexture = std::make_shared<Texture2D>(Texture2D::CreateRGBA(1, 1), sampler);
	m_gpu.yTexture = std::make_shared<Texture2D>(Texture2D::CreateRGBA(1, 1), sampler);
	m_gpu.zTexture = std::make_shared<Texture2D>(Texture2D::CreateRGBA(1, 1), sampler);
	m_gpu.pDebugBuffer = std::make_unique<GLBuffer>();

	m_pShader = nullptr;
	m_pShader = std::make_unique<Shader>();
	m_pShader->Build();
	CreateTexure(m_resolute);

}

SignedDistanceField::~SignedDistanceField()
{
}

void SignedDistanceField::Execute()
{

}

SignedDistanceField::UI::UI()
{
	glStatus = std::make_shared<GLStatus>();
	glStatus->SetBackCull(false);
}

float SignedDistanceField::CalcMinDistance(const Vector3& pos) const
{
	return m_pHalfEdge->GetBVH()->CalcMinDistance(pos).distance;
}

void SignedDistanceField::CreateSDFTexture(int resolute, Axis axis, float position, Texture2D* pTexture)
{
#ifdef GPU_DEBUG
	std::vector<unsigned char> gpuPixel;
	std::vector<float> gpuminDist(resolute * resolute);
	std::vector<float> cpuminDist(resolute * resolute);
#endif // GPU_DEBUG

	if (m_pShader) {
#ifdef GPU_DEBUG
		m_pShader->Execute(m_pHalfEdge, resolute, axis, position, pTexture, m_frequency, m_gpu.pDebugBuffer.get());
		pTexture->GetPixel(gpuPixel);
		m_gpu.pDebugBuffer->GetBufferData(gpuminDist);
#else
		m_pShader->Execute(m_pHalfEdge, resolute, axis, position, pTexture, m_frequency, nullptr);
		return;
#endif // GPU_DEBUG

	}

	auto bdb = m_pHalfEdge->GetBoundBox();
	auto diag = bdb.Max() - bdb.Min();
	auto pitch = diag / (float)resolute;
	auto halfPitch = 0.5f * pitch;
	std::vector<unsigned char> image(resolute * resolute * 4);
	Vector3 pixelPos;
	auto maxLength = bdb.MaxLength();
	
	if (axis == Axis::X) { pixelPos.x = position; }
	if (axis == Axis::Y) { pixelPos.y = position; }
	if (axis == Axis::Z) { pixelPos.z = position; }

	for (int i = 0; i < resolute; i++) {
		if (axis == Axis::X) { pixelPos.y = bdb.Min().y + (i * pitch.y) + halfPitch.y; }
		if (axis == Axis::Y) { pixelPos.z = bdb.Min().z + (i * pitch.z) + halfPitch.z; }
		if (axis == Axis::Z) { pixelPos.x = bdb.Min().x + (i * pitch.x) + halfPitch.x; }

		for (int j = 0; j < resolute; j++) {
			if (axis == Axis::X) { pixelPos.z =  bdb.Min().z + (j * pitch.z) + halfPitch.z; }
			if (axis == Axis::Y) { pixelPos.x =  bdb.Min().x + (j * pitch.x) + halfPitch.x; }
			if (axis == Axis::Z) { pixelPos.y =  bdb.Min().y + (j * pitch.y) + halfPitch.y; }
			auto pixel = j + (i * resolute);

			auto minDist = CalcMinDistance(pixelPos);
#ifdef GPU_DEBUG
			cpuminDist[pixel] = minDist;
#endif // GPU_DEBUG

			auto dist = abs(mod(minDist, m_frequency) - (m_frequency * 0.5f));
			if (dist < 0.1f) {
				image[4 * pixel] = 255;
				image[4 * pixel + 1] = 255;
				image[4 * pixel + 2] = 255;
				image[4 * pixel + 3] = 255;
			} else {
				image[4 * pixel] = 0;
				image[4 * pixel + 1] = 0;
				image[4 * pixel + 2] = 0;
				image[4 * pixel + 3] = 255;
			}
		}
	}

#ifdef GPU_DEBUG
	for (int i = 0; i < cpuminDist.size(); i++) {
		if (std::abs(gpuminDist[i] - cpuminDist[i]) > 0.0001) {
			int a = 0;
		}
	}
#endif // GPU_DEBUG


	if (!m_pShader) {
		pTexture->Build(resolute, resolute, image.data());
	}
}


void SignedDistanceField::CreateTexure(int resolute)
{
	if (m_gpu.xTexture->Size().x == resolute) { return; }
	m_gpu.xTexture->Resize(resolute, resolute);
	m_gpu.yTexture->Resize(resolute, resolute);
	m_gpu.zTexture->Resize(resolute, resolute);
	m_gpu.pDebugBuffer->Create(resolute * resolute, sizeof(float));
	m_gpu.pDebugBuffer->SetData(0);
}
void SignedDistanceField::ShowUI(RenderNode* pNode, UIContext& ui)
{
	
	const char* resolutions[] = { "4", "16", "32", "64", "128", "256", "512", "1024" };
	if (ImGui::Combo("Resolution", &m_ui.resolute, resolutions, IM_ARRAYSIZE(resolutions))) {
		if (m_ui.resolute == 0) { m_resolute = 4; } 
		else if (m_ui.resolute == 1) { m_resolute = 16; } 
		else if (m_ui.resolute == 2) { m_resolute = 32; } 
		else if (m_ui.resolute == 3) { m_resolute = 64; }
		else if (m_ui.resolute == 4) { m_resolute = 128; }
		else if (m_ui.resolute == 5) { m_resolute = 256; }
		else if (m_ui.resolute == 6) { m_resolute = 512; }
		else if (m_ui.resolute == 7) { m_resolute = 1024; }
		CreateTexure(m_resolute);
	}

	if (ImGui::Checkbox("VisibleX", &m_ui.xPlane.visible)) {
		if (!m_ui.xPlane.visible) {
			m_pHalfEdge->RemoveNode("XPlane");
		}
	}
	const auto& bdb = m_pHalfEdge->GetBoundBox();
	if (m_ui.xPlane.visible) {
		if (ImGui::SliderFloat("XPlane", &m_ui.xPlane.position, bdb.Min().x, bdb.Max().x, "%lf", 1.0f)) {
			std::shared_ptr<Primitive> pPlane = std::make_shared<PlanePrimitive>(bdb.Min(), bdb.Max(), m_ui.xPlane.position, PlanePrimitive::X, true);
			CreateSDFTexture(m_resolute, Axis::X, m_ui.xPlane.position, m_gpu.xTexture.get());
			auto pNode = std::make_shared<PrimitiveNode>("XPlane", pPlane, m_gpu.xTexture);
			pNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pNode->SetPickTarget(true);
			pNode->SetGLStatus(m_ui.glStatus);
			m_pHalfEdge->AddNode(pNode);
		}
	}

	if (ImGui::Checkbox("VisibleY", &m_ui.yPlane.visible)) {
		if (!m_ui.yPlane.visible) {
			m_pHalfEdge->RemoveNode("YPlane");
		}
	}
	if (m_ui.yPlane.visible) {
		if (ImGui::SliderFloat("YPlane", &m_ui.yPlane.position, bdb.Min().y, bdb.Max().y, "%lf", 1.0f)) {
			std::shared_ptr<Primitive> pPlane = std::make_shared<PlanePrimitive>(bdb.Min(), bdb.Max(), m_ui.yPlane.position, PlanePrimitive::Y, true);
			CreateSDFTexture(m_resolute, Axis::Y, m_ui.yPlane.position, m_gpu.yTexture.get());
			auto pNode = std::make_shared<PrimitiveNode>("YPlane", pPlane, m_gpu.yTexture);
			pNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pNode->SetGLStatus(m_ui.glStatus);
			pNode->SetPickTarget(true);
			m_pHalfEdge->AddNode(pNode);
		}
	}

	if (ImGui::Checkbox("VisibleZ", &m_ui.zPlane.visible)) {
		if (!m_ui.zPlane.visible) {
			m_pHalfEdge->RemoveNode("ZPlane");
		}
	}
	
	if (m_ui.zPlane.visible) {
		if (ImGui::SliderFloat("ZPlane", &m_ui.zPlane.position, bdb.Min().z, bdb.Max().z, "%lf", 1.0f)) {
			std::shared_ptr<Primitive> pPlane = std::make_shared<PlanePrimitive>(bdb.Min(), bdb.Max(), m_ui.zPlane.position, PlanePrimitive::Z, true);
			CreateSDFTexture(m_resolute, Axis::Z, m_ui.zPlane.position, m_gpu.zTexture.get());
			auto pNode = std::make_shared<PrimitiveNode>("ZPlane", pPlane, m_gpu.zTexture);
			pNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pNode->SetGLStatus(m_ui.glStatus);
			pNode->SetPickTarget(true);
			m_pHalfEdge->AddNode(pNode);
		}
	}
}

ShaderPath SignedDistanceField::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("algorithm/bvh.h");
#ifdef GPU_DEBUG
	path.extension[SHADER_PROGRAM_COMPUTE].push_back("#define DEBUG\n");
#endif // GPU_DEBUG

	path.shader[SHADER_PROGRAM_COMPUTE] = "algorithm/signedDistanceField.comp";
	return path;
}

void SignedDistanceField::Shader::FetchUniformLocation()
{
	m_minBox = GetUniformLocation("u_minBox");
	m_maxBox = GetUniformLocation("u_maxBox");
	m_pitch = GetUniformLocation("u_pitch");
	m_position = GetUniformLocation("u_position");
	m_axis = GetUniformLocation("u_axis");
	m_resolute = GetUniformLocation("u_resolute");
	m_maxTriangle = GetUniformLocation("u_maxTriangle");
	m_frequency = GetUniformLocation("u_frequency");
	m_model = GetUniformLocation("u_Model");

}

void SignedDistanceField::Shader::Execute(HalfEdgeNode* pNode, int resolute, Axis axis, float position, Texture2D* pTexture, float frequency, GLBuffer* pDebugBuffer)
{
	auto bdb = pNode->GetBoundBox();
	auto diag = bdb.Max() - bdb.Min();
	auto pitch = diag / (float)resolute;
	Use();
	BindUniform(m_minBox, bdb.Min());
	BindUniform(m_maxBox, bdb.Max());
	BindUniform(m_pitch, pitch);
	BindUniform(m_position, position);
	BindUniform(m_axis, (int)axis);
	BindUniform(m_resolute, resolute);
	BindUniform(m_maxTriangle, (int)pNode->GetData()->GetFaceNum());
	BindUniform(m_frequency, frequency);
	BindUniform(m_model, pNode->GetMatrix());
	BindShaderStorage(0, pNode->GetPositionGpu()->Handle());
	BindShaderStorage(1, pNode->GetFaceIndexGpu()->Handle());
	BindShaderStorage(2, pNode->GetBVHGpu()->Handle());
	BindTexture(3, pTexture, GL_WRITE_ONLY);
#ifdef GPU_DEBUG
	BindShaderStorage(4, pDebugBuffer->Handle());
#endif // GPU_DEBUG

	Dispatch(GetDispatchNum2D(Vector2i(resolute,resolute)));
	BarrierImage();
	UnUse();
}

SignedDistanceField::SDFData SignedDistanceField::CreateSDFData(int resolute)
{
	SDFData data;
	data.m_value.resize(resolute);
	for (int i = 0; i < resolute; ++i) {
		data.m_value[i].resize(resolute);
		for (int j = 0; j < resolute; ++j) {
			data.m_value[i][j].resize(resolute);
		}
	}
	return data;
}
}

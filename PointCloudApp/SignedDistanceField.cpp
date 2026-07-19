#include "SignedDistanceField.h"
#include "HalfEdgeNode.h"
#include "Primitives.h"
#include "PrimitiveNode.h"
#include "Texture.h"
#include "Utility.h"
#include "FileUtility.h"
#include "Voxelizer.h"	
#include "ProcessExecutor.h"
#define GPU_DEBUG
namespace KI
{
SignedDistanceField::SignedDistanceField(HalfEdgeNode* pHalfEdge)
	: m_pHalfEdge(pHalfEdge)
	, m_resolute(64)
	, m_frequency(2.0f)
{
	auto sampler = Texture::Sampler(Texture::Sampler::FILTER::NEAREST);
	m_gpu.xTexture = std::make_shared<Texture2D>(Texture2D::CreateRGBA(1, 1), sampler);
	m_gpu.yTexture = std::make_shared<Texture2D>(Texture2D::CreateRGBA(1, 1), sampler);
	m_gpu.zTexture = std::make_shared<Texture2D>(Texture2D::CreateRGBA(1, 1), sampler);

	m_pShader = nullptr;
	m_pShader = std::make_unique<Shader>(Shader::Type::TO_IMAGE);
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


bool SignedDistanceField::DebugSDFCPUGPU(int resolute)
{
	auto gpuminDist = CreateSDFData(resolute);
	auto bdb = m_pHalfEdge->GetBoundBox();
	auto diag = glm::length(bdb.Max() - bdb.Min());
	auto pitch = diag / (float)resolute;
	auto halfPitch = 0.5f * pitch;
	auto maxLength = bdb.MaxLength();
	for (int i = 0; i < resolute; i++)
	for (int j = 0; j < resolute; j++) 
	for (int k = 0; k < resolute; k++) {
		auto pixelPos = m_pHalfEdge->GetBoundBox().Min() + Vector3(i, j, k) * pitch + halfPitch;
		auto minDist = CalcMinDistance(pixelPos);
		if(std::abs(gpuminDist.GetData(Vector3i(i,j,k)) - minDist) < 0.001f) {
			return false;
		}
	}
}

void SignedDistanceField::CreateTexure(int resolute)
{
	if (m_gpu.xTexture->Size().x == resolute) { return; }
	m_gpu.xTexture->Resize(resolute, resolute);
	m_gpu.yTexture->Resize(resolute, resolute);
	m_gpu.zTexture->Resize(resolute, resolute);

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
			m_pShader->Execute(m_pHalfEdge, m_resolute, Axis::X, m_ui.xPlane.position, m_gpu.xTexture.get(), m_frequency, nullptr);
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
			m_pShader->Execute(m_pHalfEdge, m_resolute, Axis::Y, m_ui.yPlane.position, m_gpu.yTexture.get(), m_frequency, nullptr);
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
			m_pShader->Execute(m_pHalfEdge, m_resolute, Axis::Z, m_ui.zPlane.position, m_gpu.zTexture.get(), m_frequency, nullptr);
			auto pNode = std::make_shared<PrimitiveNode>("ZPlane", pPlane, m_gpu.zTexture);
			pNode->SetMatrix(m_pHalfEdge->GetMatrix());
			pNode->SetGLStatus(m_ui.glStatus);
			pNode->SetPickTarget(true);
			m_pHalfEdge->AddNode(pNode);
		}
	}

	if(ImGui::Button("CalcAI"))
	{
		auto voxelF = CreateSDFData(m_resolute);
		UpdateInnerOuter(voxelF);
		auto trainPath = FileUtility::GetCacheDirectory() + "\\sdfTrain" + StringUtility::ToString(m_resolute) + ".bin";
		VoxelF::Serializer::Save(trainPath, voxelF);
		AIProcessor::Instance().SendCommand("SDFAI --train " + trainPath);

		std::vector<float> predictData;
		for(int i = 0; i < voxelF.GetResolute().x; i++)
		for(int j = 0; j < voxelF.GetResolute().y; j++) 
		for(int k = 0; k < voxelF.GetResolute().z; k++) {
			auto value = voxelF.GetCenter(Vector3i(k, j, i));
			predictData.push_back(value.x);
			predictData.push_back(value.y);
			predictData.push_back(value.z);
		}

		auto inputPath = FileUtility::GetCacheDirectory() + "\\sdfInput.bin";
		auto outputPath = FileUtility::GetCacheDirectory() + "\\sdfOutput.bin";
		FileWriter writer;
		writer.Open(inputPath, true);
		writer.WriteBinary(predictData);
		writer.Close();
		AIProcessor::Instance().SendCommand("SDFAI --predict " + inputPath + " " + outputPath);

		FileReader reader;
		reader.Open(outputPath, true);
		auto outputData = reader.ReadVector<float>((int)predictData.size());
		reader.Close();

		float _max = outputData[0];
		float _min = outputData[0];
		for(size_t i = 1; i < outputData.size(); i++) {
			_max = std::max(_max, outputData[i]);
			_min = std::min(_min, outputData[i]);
		}
		Vector<Vector3> position;
		Vector<Vector3> color;
		for(size_t i = 0; i < voxelF.GetResolute().x; i++) 
		for(size_t j = 0; j < voxelF.GetResolute().y; j++) 
		for(size_t k = 0; k < voxelF.GetResolute().z; k++) {
			position.push_back(voxelF.GetCenter(Vector3i(k, j, i)));
			auto sdf = outputData[i * voxelF.GetResolute().x * voxelF.GetResolute().y + j * voxelF.GetResolute().x + k];
			color.push_back(ColorUtility::CreatePseudo(sdf, _min, _max));
			
			//if(voxelF.GetData(Vector3i(k, j, i)) < 0.0f) {
			//	position.push_back(voxelF.GetCenter(Vector3i(k, j, i)));
			//	color.push_back(voxelF.GetData(Vector3i(k, j, i)) < 0.0f ? ColorUtility::CreatePrimary(1) : ColorUtility::CreatePrimary(2));
			//}
		}
		
		{
			auto pPrimitive = std::make_shared<Primitive>();
			pPrimitive->SetPosition(std::move(position));
			pPrimitive->SetColor(std::move(color));
			pPrimitive->SetType(GL_POINTS);
			auto pNode = std::make_shared<PrimitiveNode>("SDFAI", pPrimitive);
			pNode->SetMatrix(m_pHalfEdge->GetMatrix());
			m_pHalfEdge->AddNode(pNode);
		}

		{
			auto sdfVoxel = VoxelF(voxelF.GetResolute(), voxelF.GetBDB(), std::move(outputData));
			DualContouring dual;
			auto mesh = dual.CreateMesh(sdfVoxel);
			auto pPrimitive = std::make_shared<Primitive>();
			pPrimitive->SetPosition(std::move(mesh.position));
			pPrimitive->SetNormal(std::move(mesh.normal));
			pPrimitive->SetIndex(std::move(mesh.indices));
			pPrimitive->SetType(GL_TRIANGLES);
			auto pNode = std::make_shared<PrimitiveNode>("SDFAI_DualContouring", pPrimitive);
			pNode->SetMatrix(m_pHalfEdge->GetMatrix());
			m_pHalfEdge->AddNode(pNode);
		}



		
	}
}

Vector3i SignedDistanceField::Shader::GetLocalThreadNum() const
{
	if(m_type == Type::FETCH) {
		return Vector3i(1024, 1, 1);
	} else if (m_type == Type::TO_IMAGE) {
		return Vector3i(32, 32, 1);
	} else {
		return Vector3i(1, 1, 1);
	}
}
ShaderPath SignedDistanceField::Shader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("algorithm/bvh.h");
	if (m_type == Type::FETCH) {
		path.extension[SHADER_PROGRAM_COMPUTE].push_back("#define FETCH_SDF\n");
	}
	
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
void SignedDistanceField::Shader::Execute(HalfEdgeNode* pNode, int resolute, GLBuffer* pBuffer)
{
	Execute(pNode, resolute, Axis::X, 0.0f, nullptr, 0.0f, pBuffer);
}
void SignedDistanceField::Shader::Execute(HalfEdgeNode* pNode, int resolute, Axis axis, float position, Texture2D* pTexture, float frequency, GLBuffer* pSDFBuffer)
{
	auto maxBDB = pNode->GetBoundBox().MaxLength();
	auto pitch = maxBDB / (float)resolute;
	auto min = pNode->GetBoundBox().Min();
	auto max = min + maxBDB;
	auto bdb = BDB(min, max);
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
	if(m_type == Shader::Type::FETCH) {
		BindShaderStorage(4, pSDFBuffer->Handle());
		Dispatch(GetDispatchNum1D(resolute * resolute * resolute));
	} else {
		BindTexture(3, pTexture, GL_WRITE_ONLY);
		Dispatch(GetDispatchNum2D(Vector2i(resolute, resolute)));
	}

	BarrierImage();
	UnUse();
}

VoxelF SignedDistanceField::CreateSDFData(int resolute)
{
	auto pShader = std::make_unique<Shader>(Shader::Type::FETCH);
	pShader->Build();
	auto pBuffer = std::make_unique<GLBuffer>();
	pBuffer->Create(resolute * resolute * resolute, sizeof(float));
	pShader->Execute(m_pHalfEdge, resolute, pBuffer.get());
	std::vector<float> gpuminDist(resolute * resolute * resolute);
	pBuffer->GetBufferData(gpuminDist);
	auto maxLength = m_pHalfEdge->GetBoundBox().MaxLength();
	auto min = m_pHalfEdge->GetBoundBox().Min();
	auto max = min + Vector3(maxLength, maxLength, maxLength);
	return VoxelF(Vector3i(resolute, resolute, resolute), BDB(min,max), std::move(gpuminDist));
}

void SignedDistanceField::UpdateInnerOuter(VoxelF& voxel)
{
	auto voxelizer = Voxelizer(m_pHalfEdge);
	voxelizer.Execute(voxel.GetResolute().x);
	auto label = voxelizer.CreateLabel();
	std::array<int, 4> labelCount = { 0, 0, 0, 0 };
	for(int i = 0; i < voxel.GetResolute().x; i++) 
	for(int j = 0; j < voxel.GetResolute().y; j++) 
	for(int k = 0; k < voxel.GetResolute().z; k++) {
		if(label[i][j][k] == VOXEL_LABEL_INNER) {
			voxel.SetData(Vector3i(i,j,k), -voxel.GetData(Vector3i(i,j,k)));
		}else if(label[i][j][k] == VOXEL_LABEL_OUTER) {
			voxel.SetData(Vector3i(i,j,k), voxel.GetData(Vector3i(i,j,k)));
		} else {
			voxel.SetData(Vector3i(i,j,k), 0.0f);
		}
		labelCount[label[i][j][k]]++;
	}

	std::cout << "Unknown: " << labelCount[VOXEL_LABEL_UNKNOWN] << std::endl;
	std::cout << "Boundary: " << labelCount[VOXEL_LABEL_BOUNDARY] << std::endl;
	std::cout << "Inner: " << labelCount[VOXEL_LABEL_INNER] << std::endl;
	std::cout << "Outer: " << labelCount[VOXEL_LABEL_OUTER] << std::endl;	


}
}
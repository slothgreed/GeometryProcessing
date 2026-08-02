#include "VolumeNode.h"
#include "SimpleShader.h"
#include "PostEffect.h"
#include "ShaderTable.h"
namespace KI
{

VolumeNode::VolumeImageShader::VolumeImageShader()
{
}
VolumeNode::VolumeImageShader::~VolumeImageShader()
{
}

ShaderPath VolumeNode::VolumeImageShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "volume\\volumeTexture.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "volume\\volumeTexture.frag";
	return path;

}

void VolumeNode::VolumeImageShader::FetchUniformLocation()
{
	m_uModel = GetUniformLocation("u_Model");
	m_uTexture = GetUniformLocation("u_texture3D");
	m_uPosition = GetUniformLocation("u_Position");
}

void VolumeNode::VolumeImageShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
}

void VolumeNode::VolumeImageShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &value[0][0]);
}
void VolumeNode::VolumeImageShader::SetPosition(GLBuffer* pPosition)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pPosition));
	glBindVertexBuffer(ATTRIB_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
}

void VolumeNode::VolumeImageShader::SetPosition(PlanePrimitive::Axis axis, float position)
{
	BindUniform(m_uPosition, Vector4((float)axis, position, 0.0f, 0.0f));
}

void VolumeNode::VolumeImageShader::SetTexture(Texture* pTexture)
{
	glBindTextureUnit(0, pTexture->Handle());
	glUniform1i(m_uTexture, 0);
}
void VolumeNode::VolumeImageShader::SetTexcoord(GLBuffer* pTexcoord)
{
	SetVertexFormat(VertexFormat(ATTRIB_TEXCOORD, pTexcoord));
	glBindVertexBuffer(ATTRIB_TEXCOORD, pTexcoord->Handle(), 0, pTexcoord->SizeOfData());
}

ShaderPath VolumeNode::VolumeRayCastShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.header.push_back("volume\\voxel.h");
	path.shader[SHADER_PROGRAM_VERTEX] = "volume\\volumeRayCast.vert";
	path.shader[SHADER_PROGRAM_FRAG] = "volume\\volumeRayCast.frag";
	return path;
}
void VolumeNode::VolumeRayCastShader::FetchUniformLocation()
{
	m_uModel = GetUniformLocation("u_Model");
	m_uStepSize = GetUniformLocation("u_StepSize");
	m_uTexture = GetUniformLocation("u_texture3D");
}

void VolumeNode::VolumeRayCastShader::SetTexture(Texture* pTexture)
{
	glBindTextureUnit(0, pTexture->Handle());
	glUniform1i(m_uTexture, 0);
	OUTPUT_GLERROR;
}

void VolumeNode::VolumeRayCastShader::SetCamera(const GLBuffer* pBuffer)
{
	BindShaderStorage(0, pBuffer->Handle());
}

void VolumeNode::VolumeRayCastShader::SetPosition(GLBuffer* pPosition)
{
	SetVertexFormat(VertexFormat(ATTRIB_POSITION, pPosition));
	glBindVertexBuffer(ATTRIB_POSITION, pPosition->Handle(), 0, pPosition->SizeOfData());
}
void VolumeNode::VolumeRayCastShader::SetModel(const Matrix4x4& value)
{
	BindUniform(m_uModel, value);
}

void VolumeNode::VolumeRayCastShader::SetVoxel(const GLBuffer* pBuffer)
{
	BindShaderStorage(1, pBuffer->Handle());
}



VolumeNode::VolumeNode(Unique<VoxelU16>&& pVoxel)
	: RenderNode("Volume")
	, m_pVolumeImageShader(nullptr)
	, m_pVoxel(std::move(pVoxel))
{
	auto bdb = BDB();
	bdb.Add(Vector3(0, 0, 0));
	bdb.Add(Vector3(1, 1, 1));
	SetBoundBox(bdb);
}

VolumeNode::~VolumeNode()
{
}

void VolumeNode::BuildResource()
{
	if (m_gpu.pBDBLine) { return; }
	m_pVolumeImageShader = std::make_unique<VolumeNode::VolumeImageShader>();
	m_pVolumeImageShader->Build();


	m_pTexture3D = std::make_unique<Texture3D>();
	m_pTexture3D->Build(m_pVoxel->GetResolute(), CreateGrayScale(*m_pVoxel).data());

	auto cube = Cube::CreateLine(GetBoundBox().Min(), GetBoundBox().Max());
	m_gpu.pBDBLine = std::make_unique<GLBuffer>();
	m_gpu.pBDBLineIndex = std::make_unique<GLBuffer>();

	m_gpu.pBDBLine->Create(cube.Position());
	m_gpu.pBDBLineIndex->Create(cube.Index());

	m_gpu.pPlane = std::make_unique<GLBuffer>();
	m_gpu.pPlaneTex = std::make_unique<GLBuffer>();
	m_gpu.pPlaneIndex = std::make_unique<GLBuffer>();

	auto plane = PlanePrimitive(GetBoundBox().Min(), GetBoundBox().Max(), 0.0f, KI::PlanePrimitive::X, true);
	m_gpu.pPlane->Create(plane.Position());
	m_gpu.pPlaneTex->Create(plane.Texcoord());
	m_gpu.pPlaneIndex->Create(plane.Index());

}
void VolumeNode::Draw(const DrawContext& context)
{
	if (!m_ui.visible) { return; }

	BuildResource();
	context.pResource->GL()->EnableCullFace();

	// BDB
	if (m_ui.visibleBDB)
	{
		auto pSimple = context.pResource->GetShaderTable()->GetSimpleShader();
		pSimple->Use();
		pSimple->SetCamera(context.pResource->GetCameraBuffer());
		pSimple->SetModel(GetMatrix());
		pSimple->SetColor(Vector3(0, 0, 1));
		pSimple->SetPosition(m_gpu.pBDBLine.get());
		pSimple->DrawElement(GL_LINES, m_gpu.pBDBLineIndex.get());
	}

	if (m_ui.marching.visible) {
		if (m_ui.marching.useMeshShader) {
			m_pMarchingShader->Use();
			m_pMarchingShader->SetCamera(context.pResource->GetCameraBuffer());
			m_pMarchingShader->SetModel(GetMatrix());
			m_pMarchingShader->SetVoxel(m_gpu.pVoxel.get());
			m_pMarchingShader->SetVoxelData(m_gpu.pVoxelData.get());
			m_pMarchingShader->SetThreshold(m_ui.marching.isolate);
			if (m_pMarchingShader->WithTask()) {
				m_pMarchingShader->PreDraw();
				m_pMarchingShader->DrawWithAutoTask(0, m_pVoxel->GetSize());
				m_pMarchingShader->BarrierSSBO();
			} else {
				m_pMarchingShader->PreDraw();
				m_pMarchingShader->Draw(0, m_pVoxel->GetSize());
			}
		} else {
			if (m_gpu.pMarchingPosition) {
				auto pFace = context.pResource->GetShaderTable()->GetFaceShader();
				pFace->Use();
				pFace->SetCamera(context.pResource->GetCameraBuffer());
				pFace->SetLight(context.pResource->GetLightBuffer());
				pFace->SetPBRResource(context.pResource->GetPBR());
				pFace->SetModel(GetMatrix());
				pFace->SetColor(Vector3(0, 0, 1));
				pFace->SetPosition(m_gpu.pMarchingPosition.get());
				pFace->SetNormal(m_gpu.pMarchingNormal.get());
				pFace->DrawArray(GL_TRIANGLES, m_gpu.pMarchingPosition.get());
			}
		}
	}

	if (m_ui.rayCast.visible) {
		if (!m_pRayCastShader) { m_pRayCastShader = std::make_unique<VolumeRayCastShader>(); m_pRayCastShader->Build(); }
		BuildVoxelResource(false);
		m_pRayCastShader->Use();
		m_pRayCastShader->SetPosition(context.pResource->GetTexturePlane()->GetPositionBuffer().get());
		m_pRayCastShader->SetModel(GetMatrix());
		m_pRayCastShader->SetVoxel(m_gpu.pVoxel.get());
		m_pRayCastShader->SetTexture(m_pTexture3D.get());
		m_pRayCastShader->DrawElement(GL_TRIANGLES, context.pResource->GetTexturePlane()->GetIndexBuffer().get());
	}

	context.pResource->GL()->DisableCullFace();
	{
		m_pVolumeImageShader->Use();
		m_pVolumeImageShader->SetPosition(m_gpu.pPlane.get());
		m_pVolumeImageShader->SetTexcoord(m_gpu.pPlaneTex.get());
		m_pVolumeImageShader->SetTexture(m_pTexture3D.get());
		if (m_ui.xPlane.visible) {
			m_pVolumeImageShader->SetPosition(PlanePrimitive::X, m_ui.xPlane.position);
			m_pVolumeImageShader->SetModel(GetMatrix() * m_ui.xPlane.matrix);
			m_pVolumeImageShader->DrawElement(GL_TRIANGLES, m_gpu.pPlaneIndex.get());
		}

		if (m_ui.yPlane.visible) {
			m_pVolumeImageShader->SetPosition(PlanePrimitive::Y, m_ui.yPlane.position);
			m_pVolumeImageShader->SetModel(GetMatrix() * m_ui.yPlane.matrix);
			m_pVolumeImageShader->DrawElement(GL_TRIANGLES, m_gpu.pPlaneIndex.get());
		}

		if (m_ui.zPlane.visible) {
			m_pVolumeImageShader->SetPosition(PlanePrimitive::Z, m_ui.zPlane.position);
			m_pVolumeImageShader->SetModel(GetMatrix() * m_ui.zPlane.matrix);
			m_pVolumeImageShader->DrawElement(GL_TRIANGLES, m_gpu.pPlaneIndex.get());
		}
	}
	context.pResource->GL()->EnableCullFace();
}
void VolumeNode::ShowUI(UIContext& ui)
{
	ImGui::Checkbox("Visible", &m_ui.visible);
	if (!m_ui.visible) { return; }
	ImGui::Checkbox("VisibleBDB", &m_ui.visibleBDB);
	ImGui::Checkbox("VisibleX", &m_ui.xPlane.visible);
	if (m_ui.xPlane.visible) {
		if (ImGui::SliderFloat("XPlane", &m_ui.xPlane.position, GetBoundBox().Min().x, GetBoundBox().Max().x, "%lf", 1.0f)) {
			m_ui.xPlane.matrix = PlanePrimitive::CreateMatrix(GetBoundBox().Min(), GetBoundBox().Max(), m_ui.xPlane.position, PlanePrimitive::X);
		}
	}

	ImGui::Checkbox("VisibleY", &m_ui.yPlane.visible);
	if (m_ui.yPlane.visible) {
		if (ImGui::SliderFloat("YPlane", &m_ui.yPlane.position, GetBoundBox().Min().y, GetBoundBox().Max().y, "%lf", 1.0f)) {
			m_ui.yPlane.matrix = PlanePrimitive::CreateMatrix(GetBoundBox().Min(), GetBoundBox().Max(), m_ui.yPlane.position, PlanePrimitive::Y);
		}
	}

	ImGui::Checkbox("VisibleZ", &m_ui.zPlane.visible);
	if (m_ui.zPlane.visible) {
		if (ImGui::SliderFloat("ZPlane", &m_ui.zPlane.position, GetBoundBox().Min().z, GetBoundBox().Max().z, "%lf", 1.0f)) {
			m_ui.zPlane.matrix = PlanePrimitive::CreateMatrix(GetBoundBox().Min(), GetBoundBox().Max(), m_ui.zPlane.position, PlanePrimitive::Z);
		}
	}


	ImGui::Checkbox("VisibleMarchingCube", &m_ui.marching.visible);
	if (m_ui.marching.visible) {
		if (ImGui::Checkbox("MeshShader", &m_ui.marching.useMeshShader)) {
			if (m_ui.marching.useMeshShader) {
				if (m_pMarchingShader == nullptr) {
					m_pMarchingShader = std::make_unique<MarchingCubeShader>(m_pVoxel->GetSize(), true); m_pMarchingShader->Build();
					BuildVoxelResource(true);
				}
			}
		}
		if (ImGui::SliderFloat("MarchingThreshold", &m_ui.marching.isolate, 0, 2000)) {
			if (!m_ui.marching.useMeshShader) {
				MarchingCube marching;
				auto mesh = marching.CreateMesh(*m_pVoxel, m_ui.marching.isolate);
				m_gpu.pMarchingPosition = std::make_unique<GLBuffer>();
				m_gpu.pMarchingNormal = std::make_unique<GLBuffer>();
				m_gpu.pMarchingPosition->Create(mesh.GetPoints());
				m_gpu.pMarchingNormal->Create(mesh.GetNormals());
			}
		}
	}

	ImGui::Checkbox("VisibleRayCast", &m_ui.rayCast.visible);
	RenderNode::ShowMatrixUI(ui);
}

void VolumeNode::BuildVoxelResource(bool withData)
{
	if (!m_gpu.pVoxel) {
		m_gpu.pVoxel = std::make_unique<GLBuffer>();
		m_gpu.pVoxel->Create<VoxelGpu>(VoxelGpu(*m_pVoxel));
	}
	if (withData) {
		if (!m_gpu.pVoxelData) {
			m_gpu.pVoxelData = std::make_unique<GLBuffer>();
			m_gpu.pVoxelData->Create(m_pVoxel->GetData());
		}
	}

}


std::vector<Vector4> VolumeNode::CreateGrayScale(const VoxelU16& voxel) const
{
	std::vector<Vector4> grayScale(voxel.GetResolute().x * voxel.GetResolute().y * voxel.GetResolute().z);
	unsigned short maxValue = 0.0f;
	for (int x = 0; x < voxel.GetResolute().x; x++)
		for (int y = 0; y < voxel.GetResolute().y; y++)
			for (int z = 0; z < voxel.GetResolute().z; z++) {
				maxValue = std::max(maxValue, voxel.GetData(Vector3i(x, y, z)));
			}

	for (int x = 0; x < voxel.GetResolute().x; x++)
		for (int y = 0; y < voxel.GetResolute().y; y++)
			for (int z = 0; z < voxel.GetResolute().z; z++) {
				int index = voxel.GetIndex(x, y, z);
				float normalized = static_cast<float>(voxel.GetData(Vector3i(x, y, z))) / maxValue;
				grayScale[index].x = normalized;
				grayScale[index].y = normalized;
				grayScale[index].z = normalized;
				grayScale[index].w = 1.0f;
			}

	return grayScale;
}










VoxelNode::VoxelNode(const String& name, Unique<VoxelF>&& pVoxel)
	: RenderNode(name)
	, m_pVoxel(std::move(pVoxel))
{
	m_maxValue = m_pVoxel->GetData()[0];
	m_minValue = m_pVoxel->GetData()[0];
	for (size_t i = 0; i < m_pVoxel->GetData().size(); i++) {
		m_maxValue = std::max(m_maxValue, m_pVoxel->GetData()[i]);
		m_minValue = std::min(m_minValue, m_pVoxel->GetData()[i]);
	}
}
VoxelNode::~VoxelNode()
{

}
void VoxelNode::BuildResource()
{
	if (!m_gpu.pPosition) {
		m_gpu.pPosition = std::make_unique<GLBuffer>();
		m_gpu.pColor = std::make_unique<GLBuffer>();
	}
	
	if (m_cache.maxValue == m_ui.maxValue && m_cache.minValue == m_ui.minValue) { return; }

	Vector<Vector3> position;
	Vector<Vector3> color;
	for (size_t i = 0; i < m_pVoxel->GetResolute().x; i++)
	for (size_t j = 0; j < m_pVoxel->GetResolute().y; j++)
	for (size_t k = 0; k < m_pVoxel->GetResolute().z; k++) {
		auto data = m_pVoxel->GetData(Vector3i(i, j, k));
		if (data < m_ui.minValue || data > m_ui.maxValue) { continue; }
		position.push_back(m_pVoxel->GetCenter(Vector3i(i, j, k)));
		color.push_back(ColorUtility::CreatePseudo(m_pVoxel->GetData(Vector3i(i, j, k)), m_minValue, m_maxValue));
	}

	m_cache.maxValue = m_ui.maxValue;
	m_cache.minValue = m_ui.minValue;

	if (position.size() == 0) {
		m_gpu.pPosition->Delete();
		m_gpu.pColor->Delete();
		return;
	}
	m_gpu.pPosition->Create(position);
	m_gpu.pColor->Create(color);
}


void VoxelNode::Draw(const DrawContext& context)
{
	if (m_ui.visiblePoints) {
		BuildResource();
		if (m_gpu.pPosition->Num() != 0) {
			auto pVertexColorShader = context.pResource->GetShaderTable()->GetVertexColorShader();
			pVertexColorShader->Use();
			pVertexColorShader->SetCamera(context.pResource->GetCameraBuffer());
			pVertexColorShader->SetModel(GetMatrix());
			pVertexColorShader->SetPosition(m_gpu.pPosition.get());
			pVertexColorShader->SetColor(m_gpu.pColor.get());
			pVertexColorShader->DrawArray(GL_POINTS, m_gpu.pPosition->Num());
		}
	}
	if (m_ui.marching.visible) {
		BuildResource();
		m_pMarchingShader->Use();
		m_pMarchingShader->SetCamera(context.pResource->GetCameraBuffer());
		m_pMarchingShader->SetModel(GetMatrix());
		m_pMarchingShader->SetVoxel(m_gpu.pVoxel.get());
		m_pMarchingShader->SetVoxelData(m_gpu.pVoxelData.get());
		m_pMarchingShader->SetThreshold(m_ui.marching.isolate);
		if (m_pMarchingShader->WithTask()) {
			m_pMarchingShader->PreDraw();
			m_pMarchingShader->DrawWithAutoTask(0, m_pVoxel->GetSize());
			m_pMarchingShader->BarrierSSBO();
		} else {
			m_pMarchingShader->PreDraw();
			m_pMarchingShader->Draw(0, m_pVoxel->GetSize());
		}
	}
}
void VoxelNode::ShowUI(UIContext& ui)
{
	ImGui::Checkbox("VisiblePoints", &m_ui.visiblePoints);
	if (m_ui.visiblePoints) {
		ImGui::SliderFloat("Min", &m_ui.minValue, m_minValue, m_ui.maxValue);
		ImGui::SliderFloat("Max", &m_ui.maxValue, m_ui.minValue, m_maxValue);
	}
	ImGui::Checkbox("VisibleMarchingCube", &m_ui.marching.visible);
	if (m_ui.marching.visible) {
		if (m_pMarchingShader == nullptr) {
			m_pMarchingShader = std::make_unique<MarchingCubeShader>(m_pVoxel->GetSize(), false, DATA_TYPE::DATA_FLOAT); m_pMarchingShader->Build();
			BuildVoxelResource();
		}

		ImGui::SliderFloat("MarchingThreshold", &m_ui.marching.isolate, m_minValue, m_maxValue);
	}
}

void VoxelNode::BuildVoxelResource()
{
	if (!m_gpu.pVoxel) {
		m_gpu.pVoxel = std::make_unique<GLBuffer>();
		m_gpu.pVoxel->Create<VoxelGpu>(VoxelGpu(*m_pVoxel));
	}
	if (!m_gpu.pVoxelData) {
		m_gpu.pVoxelData = std::make_unique<GLBuffer>();
		m_gpu.pVoxelData->Create(m_pVoxel->GetData());
	}
}

}

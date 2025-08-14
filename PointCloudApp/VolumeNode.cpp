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

void VolumeNode::VolumeImageShader::SetPosition(Plane::Axis axis, float position)
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

ShaderPath VolumeNode::MarchingCubeShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.header.push_back("volume\\voxel.h");
	
	path.header.push_back("volume\\marchingCube.h");
	if (m_withTask) {
		path.shader[SHADER_PROGRAM_TASK] = "volume\\marchingCube.task";
		path.shader[SHADER_PROGRAM_MESH] = "volume\\marchingCubeWithTask.mesh";
	} else {
		path.shader[SHADER_PROGRAM_MESH] = "volume\\marchingCube.mesh";
	}
	path.shader[SHADER_PROGRAM_FRAG] = "volume\\marchingCube.frag";
	return path;
}

void VolumeNode::MarchingCubeShader::FetchUniformLocation()
{
	m_uModel = GetUniformLocation("u_Model");
	m_uThreshold = GetUniformLocation("u_threshold");
	m_uTaskNum = GetUniformLocation("u_taskNum");
}

void VolumeNode::MarchingCubeShader::SetCamera(const GLBuffer* pBuffer)
{
	BindShaderStorage(0, pBuffer->Handle());
}
void VolumeNode::MarchingCubeShader::SetVoxel(const GLBuffer* pBuffer)
{
	BindShaderStorage(1, pBuffer->Handle());
}
void VolumeNode::MarchingCubeShader::SetVoxelData(const GLBuffer* pBuffer)
{
	BindShaderStorage(2, pBuffer->Handle());
}
void VolumeNode::MarchingCubeShader::SetTriTable(const GLBuffer* pBuffer)
{
	BindShaderStorage(3, pBuffer->Handle());
}

void VolumeNode::MarchingCubeShader::SetCubeIndex(const GLBuffer* pBuffer)
{
	BindShaderStorage(4, pBuffer->Handle());
}
void VolumeNode::MarchingCubeShader::SetTaskNums(const GLBuffer* pBuffer)
{
	BindShaderStorage(5, pBuffer->Handle());
}

void VolumeNode::MarchingCubeShader::SetModel(const Matrix4x4& value)
{
	BindUniform(m_uModel, value);
}
void VolumeNode::MarchingCubeShader::SetThreshold(float threshold)
{
	BindUniform(m_uThreshold, threshold);
}

void VolumeNode::MarchingCubeShader::SetTaskNum(unsigned int taskNum)
{
	BindUniform(m_uTaskNum, taskNum);
}

VolumeNode::VolumeNode(Unique<Voxel>&& pVoxel)
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
	m_pTexture3D->Build(m_pVoxel->GetResolute(), m_pVoxel->CreateGrayScale().data());

	auto cube = Cube::CreateLine(GetBoundBox().Min(), GetBoundBox().Max());
	m_gpu.pBDBLine = std::make_unique<GLBuffer>();
	m_gpu.pBDBLineIndex = std::make_unique<GLBuffer>();

	m_gpu.pBDBLine->Create(cube.Position());
	m_gpu.pBDBLineIndex->Create(cube.Index());

	m_gpu.pPlane = std::make_unique<GLBuffer>();
	m_gpu.pPlaneTex = std::make_unique<GLBuffer>();
	m_gpu.pPlaneIndex = std::make_unique<GLBuffer>();

	auto plane = Plane(GetBoundBox().Min(), GetBoundBox().Max(), 0.0f, KI::Plane::X, true);
	m_gpu.pPlane->Create(plane.Position());
	m_gpu.pPlaneTex->Create(plane.Texcoord());
	m_gpu.pPlaneIndex->Create(plane.Index());

}
void VolumeNode::Draw(const DrawContext& context)
{
	BuildResource();
	context.pResource->GL()->EnableCullFace();

	// BDB
	{
		auto pSimple = context.pResource->GetShaderTable()->GetSimpleShader();
		pSimple->Use();
		pSimple->SetCamera(context.pResource->GetCameraBuffer());
		pSimple->SetModel(GetMatrix());
		pSimple->SetColor(Vector3(0, 0, 1));
		pSimple->SetPosition(m_gpu.pBDBLine.get());
		pSimple->DrawElement(GL_LINES, m_gpu.pBDBLineIndex.get());
		if (m_marching.GetPositionBuffer()) {
			pSimple->SetPosition(m_marching.GetPositionBuffer());
			pSimple->DrawArray(GL_TRIANGLES, m_marching.GetPositionBuffer());
		}
	}

	if (m_ui.marching.visible) {
		if (m_ui.marching.useMeshShader) {
			m_pMarchingShader->Use();
			m_pMarchingShader->SetCamera(context.pResource->GetCameraBuffer());
			m_pMarchingShader->SetModel(GetMatrix());
			m_pMarchingShader->SetVoxel(m_gpu.pVoxel.get());
			m_pMarchingShader->SetVoxelData(m_gpu.pVoxelData.get());
			m_pMarchingShader->SetTriTable(m_gpu.pTriTable.get());
			m_pMarchingShader->SetThreshold(m_ui.marching.isolate);
			if (m_pMarchingShader->WithTask()) {
				m_gpu.pTaskNum->SetData(0);
				m_gpu.pCubeIndexs->SetData(0);
				m_pMarchingShader->SetTaskNums(m_gpu.pTaskNum.get());
				m_pMarchingShader->SetTaskNum(m_pMarchingShader->GetTaskThreadNum());
				m_pMarchingShader->SetCubeIndex(m_gpu.pCubeIndexs.get());
				m_pMarchingShader->DrawWithAutoTask(0, m_pVoxel->GetSize());
				m_pMarchingShader->BarrierSSBO();
			} else {
				m_pMarchingShader->Draw(0, m_pVoxel->GetSize());
			}
		} else {
			if (m_marching.GetPositionBuffer()) {
				auto pFace = context.pResource->GetShaderTable()->GetFaceShader();
				pFace->Use();
				pFace->SetCamera(context.pResource->GetCameraBuffer());
				pFace->SetModel(GetMatrix());
				pFace->SetColor(Vector3(0, 0, 1));
				pFace->SetPosition(m_marching.GetPositionBuffer());
				pFace->SetNormal(m_marching.GetNormalBuffer());
				pFace->DrawArray(GL_TRIANGLES, m_marching.GetPositionBuffer());
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
			m_pVolumeImageShader->SetPosition(Plane::X, m_ui.xPlane.position);
			m_pVolumeImageShader->SetModel(GetMatrix() * m_ui.xPlane.matrix);
			m_pVolumeImageShader->DrawElement(GL_TRIANGLES, m_gpu.pPlaneIndex.get());
		}

		if (m_ui.yPlane.visible) {
			m_pVolumeImageShader->SetPosition(Plane::Y, m_ui.yPlane.position);
			m_pVolumeImageShader->SetModel(GetMatrix() * m_ui.yPlane.matrix);
			m_pVolumeImageShader->DrawElement(GL_TRIANGLES, m_gpu.pPlaneIndex.get());
		}

		if (m_ui.zPlane.visible) {
			m_pVolumeImageShader->SetPosition(Plane::Z, m_ui.zPlane.position);
			m_pVolumeImageShader->SetModel(GetMatrix() * m_ui.zPlane.matrix);
			m_pVolumeImageShader->DrawElement(GL_TRIANGLES, m_gpu.pPlaneIndex.get());
		}
	}
	context.pResource->GL()->EnableCullFace();
}
void VolumeNode::ShowUI(UIContext& ui)
{
	ImGui::Checkbox("VisibleX", &m_ui.xPlane.visible);
	if (m_ui.xPlane.visible) {
		if (ImGui::SliderFloat("XPlane", &m_ui.xPlane.position, GetBoundBox().Min().x, GetBoundBox().Max().x, "%lf", 1.0f)) {
			m_ui.xPlane.matrix = Plane::CreateMatrix(GetBoundBox().Min(), GetBoundBox().Max(), m_ui.xPlane.position, Plane::X);
		}
	}

	ImGui::Checkbox("VisibleY", &m_ui.yPlane.visible);
	if (m_ui.yPlane.visible) {
		if (ImGui::SliderFloat("YPlane", &m_ui.yPlane.position, GetBoundBox().Min().y, GetBoundBox().Max().y, "%lf", 1.0f)) {
			m_ui.yPlane.matrix = Plane::CreateMatrix(GetBoundBox().Min(), GetBoundBox().Max(), m_ui.yPlane.position, Plane::Y);
		}
	}

	ImGui::Checkbox("VisibleZ", &m_ui.zPlane.visible);
	if (m_ui.zPlane.visible) {
		if (ImGui::SliderFloat("ZPlane", &m_ui.zPlane.position, GetBoundBox().Min().z, GetBoundBox().Max().z, "%lf", 1.0f)) {
			m_ui.zPlane.matrix = Plane::CreateMatrix(GetBoundBox().Min(), GetBoundBox().Max(), m_ui.zPlane.position, Plane::Z);
		}
	}


	ImGui::Checkbox("VisibleMarchingCube", &m_ui.marching.visible);
	if (m_ui.marching.visible) {
		if (ImGui::Checkbox("MeshShader", &m_ui.marching.useMeshShader)) {
			if (m_ui.marching.useMeshShader) {
				if (m_pMarchingShader == nullptr) {
					m_pMarchingShader = std::make_unique<MarchingCubeShader>(true); m_pMarchingShader->Build();
					BuildVoxelResource(true);
					m_gpu.pTriTable = std::make_unique<GLBuffer>();
					m_gpu.pTriTable->Create(m_marching.CreateFlattenTriangleTable());
					Vector<Vector2> value(m_pVoxel->GetSize(), Vector2(0));
					m_gpu.pCubeIndexs = std::make_unique<GLBuffer>();
					m_gpu.pCubeIndexs->Create(value);
					Vector<unsigned int> taskNum(CeilDiv(m_pVoxel->GetSize(), m_pMarchingShader->GetTaskThreadNum()), 0);
					m_gpu.pTaskNum = std::make_unique<GLBuffer>();
					m_gpu.pTaskNum->Create(taskNum);
				}
			}
		}
		if (ImGui::SliderFloat("MarchingThreshold", &m_ui.marching.isolate, 0, 2000)) {
			if (!m_ui.marching.useMeshShader) {
				m_marching.Build(*m_pVoxel, m_ui.marching.isolate);
			}
		}
	}

	ImGui::Checkbox("VisibleRayCast", &m_ui.rayCast.visible);
}

void VolumeNode::BuildVoxelResource(bool withData)
{
	if (!m_gpu.pVoxel) {
		m_gpu.pVoxel = std::make_unique<GLBuffer>();
		m_gpu.pVoxel->Create<Voxel::Gpu>(m_pVoxel->CreateGpuInfo());
	}
	if (withData) {
		if (!m_gpu.pVoxelData) {
			m_gpu.pVoxelData = std::make_unique<GLBuffer>();
			m_gpu.pVoxelData->Create(m_pVoxel->GetDatas());
		}
	}

}
}

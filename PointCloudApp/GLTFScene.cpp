#include "GLTFScene.h"
#include "GLBuffer.h"
#include "GLTFShader.h"
#include "GLTFSceneUpdater.h"
namespace KI
{
bool useGpu = false;
GLTFScene::~GLTFScene()
{
	RELEASE_INSTANCE(m_pChannelGpuUpdater);
	RELEASE_INSTANCE(m_pMatrixGpuUpdater);
	RELEASE_INSTANCE(m_gpu.nodeBuffer);
	RELEASE_INSTANCE(m_gpu.materialBuffer);
	RELEASE_INSTANCE(m_gpu.skinBuffer);
	RELEASE_VECTOR(m_gpu.channelBuffer);
	RELEASE_VECTOR(m_gpu.samplerBuffer);
}

void GLTFScene::Initialize()
{
	if (useGpu) {
		m_pMatrixGpuUpdater = new GLTFSceneMatrixUpdaterOnGpu();
		m_pChannelGpuUpdater = new GLTFChannelUpdaterOnGpu();
		m_pSkinGpuUpdater = new GLTFSkinUpdaterOnGpu();
	}
	InitMatrix();
	InitAnimation();
	InitSkeleton();
}

void GLTFScene::InitSkeleton()
{
	if (m_skins.size() != 0) {
		auto gpuMemory = GLTFSkin::CreateGpuObject(m_skins);
		m_gpu.skinBuffer = new GLBuffer();
		m_gpu.skinBuffer->Create<float>(gpuMemory.data);
	}
}
void GLTFScene::InitAnimation()
{

	for (size_t i = 0; i < m_animation.size(); i++) {
		{
			auto channel = m_animation[i].CreateChannelGpuObject();

			auto pBuffer = new GLBuffer();
			pBuffer->Create<GLTFAnimation::ChannelGpuObject>(channel);
			m_gpu.channelBuffer.push_back(pBuffer);

			m_gpu.channel.push_back(std::move(channel));
		}

		{
			auto sampler = m_animation[i].CreateSamplerGpuObject();

			auto pBuffer = new GLBuffer();
			pBuffer->Create<GLTFAnimation::SamplerGpuObject>(sampler);
			m_gpu.samplerBuffer.push_back(pBuffer);

			m_gpu.sampler.push_back(std::move(sampler));
		}
	}

}

void GLTFScene::InitMatrix()
{
	if (m_gpu.nodeBuffer == nullptr) {
		m_gpu.nodeBuffer = new GLBuffer();
		m_gpu.node = GLTFNode::CreateGpuObject(m_nodes, m_skins);
		m_gpu.nodeBuffer->Create<GLTFNode::GpuObject>(m_gpu.node);
	}

	if (m_pMatrixGpuUpdater) {
		m_pMatrixGpuUpdater->Initialize(m_nodes);
		m_pMatrixGpuUpdater->Execute(m_gpu.nodeBuffer);
	}

	UpdateMatrix();
}
void GLTFScene::CreateMaterialBuffer()
{
	if (m_gpu.materialBuffer != nullptr) { return; }
	m_gpu.materialBuffer = new GLBuffer();
	m_gpu.materialBuffer->Create<GLTFMaterial>(m_material);
}

void GLTFScene::DrawNode(const DrawContext& context)
{
	if (!m_pShader) {
		m_pShader = new GLTFShader();
		m_pShader->Build();
		CreateMaterialBuffer();
	}
	m_pShader->Use();
	m_pShader->SetCamera(context.gpuCamera);
	m_pShader->SetModel(GetMatrix());
	m_pShader->SetNodeBuffer(m_gpu.nodeBuffer);
	m_pShader->SetMaterialBuffer(m_gpu.materialBuffer);
	if (m_gpu.skinBuffer) {
		m_pShader->SetSkinBuffer(m_gpu.skinBuffer);
	}
	int counter = 0;
	for (const auto& node : m_nodes) {
		if (node.GetMeshId() == -1) { continue; }
		const auto& mesh = m_meshes[node.GetMeshId()];
		if (mesh.GetBufferIndex() == -1) { continue; }
		const auto& meshBuffer = m_meshBuffer[mesh.GetBufferIndex()];
		m_pShader->SetVertexBuffer(meshBuffer.pVertex.get(), meshBuffer.format);
		m_pShader->SetIndexBuffer(meshBuffer.pIndex.get());
		for (const auto& primitive : mesh.GetPrimitives()) {
			m_pShader->BindBufferIndex(node.GetIndex(), primitive.materialIndex);
			const auto& material = m_material[primitive.materialIndex];
			if (material.baseTexture != -1) {
				m_pShader->BindBaseColor(*m_texture[material.baseTexture]);
			}

			if (material.normalTexture != -1) {
				m_pShader->BindNormal(*m_texture[material.normalTexture]);
			}

			if (material.roughnessTexture != -1) {
				m_pShader->BindRoughness(*m_texture[material.roughnessTexture]);
			}
			
			m_pShader->DrawElement(primitive, meshBuffer.pIndex->DataType());
		}
	}
}

void GLTFScene::UpdateData(float time)
{
	//time = 0.5f;
	UpdateAnimation(time);
	UpdateMatrix();
	UpdateSkin();
}

void GLTFScene::UpdateMatrix()
{
	if (m_pMatrixGpuUpdater) {
		m_pMatrixGpuUpdater->Execute(m_gpu.nodeBuffer);
	} else {
		GLTFNode::UpdateMatrix(m_roots, m_nodes);
		m_gpu.node = GLTFNode::CreateGpuObject(m_nodes, m_skins);
		m_gpu.nodeBuffer->BufferSubData<GLTFNode::GpuObject>(0, m_gpu.node);
	}
}

void GLTFScene::UpdateAnimation(float time)
{
	if (m_pChannelGpuUpdater) {
		for (size_t i = 0; i < m_animation.size(); i++) {
			m_pChannelGpuUpdater->Execute(
				m_gpu.nodeBuffer,
				m_gpu.channelBuffer[i],
				m_gpu.samplerBuffer[i],
				time);
		}
	} else {
		GLTFAnimation::Update(m_animation, m_nodes, time);
	}

}

void GLTFScene::UpdateSkin()
{
	if (m_skins.size() == 0) { return; }

	if (m_pSkinGpuUpdater) {
		for (size_t i = 0; i < m_skins.size(); i++) {
			m_pSkinGpuUpdater->Execute(
				m_gpu.nodeBuffer,
				m_gpu.skinBuffer,
				m_skins[i].GetJointNodeIndex().size());
		}
	} else {
		GLTFSkin::Update(m_skins, m_nodes);
		auto gpuData = GLTFSkin::CreateGpuObject(m_skins);
		m_gpu.skinBuffer->BufferSubData<float>(0, gpuData.data);
	}
}
}
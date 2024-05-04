#include "GLTFStruct.h"
#include "Utility.h"
#include "Texture.h"
#include "GLTFLoader.h"
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>
#include "GLTFShader.h"
#include "GLBuffer.h"
#include "GLUtility.h"
#include "GLTFSceneUpdater.h"
namespace KI
{
GLTFScene::~GLTFScene()
{
	RELEASE_INSTANCE(m_pMaterials);
	RELEASE_INSTANCE(m_pMatrixGpuUpdater);
	RELEASE_INSTANCE(m_pNodeBuffer);
}
void GLTFScene::Initialize()
{
	if (m_pNodeBuffer == nullptr) {
		m_pNodeBuffer = new GLBuffer();
		m_gpu.node = GLTFNode::CreateGpuObject(m_nodes, m_skins);
		m_pNodeBuffer->Create<GLTFNode::GpuObject>(m_gpu.node);
	}

	//m_pMatrixGpuUpdater = new GLTFSceneMatrixUpdaterOnGpu();
	//if (m_pMatrixGpuUpdater) {
	//	m_pMatrixGpuUpdater->Initialize(m_nodes);
	//	m_pMatrixGpuUpdater->Execute(m_pNodeBuffer);
	//}


	UpdateMatrix();
}
void GLTFScene::CreateMaterialBuffer()
{
	if (m_pMaterials != nullptr) { return; }
	m_pMaterials = new GLBuffer();
	m_pMaterials->Create<GLTFMaterial>(m_material);
	m_pMaterials->BufferSubData<GLTFMaterial>(0, m_material);
}

void GLTFScene::Draw(const Matrix4x4& proj, const Matrix4x4& view)
{
	if (!m_pShader) {
		m_pShader = new GLTFShader();
		m_pShader->Build();
		CreateMaterialBuffer();
	}
	m_pShader->Use();
	m_pShader->SetViewProj(proj * view);
	m_pShader->SetNodeBuffer(m_pNodeBuffer);
	m_pShader->SetMaterialBuffer(m_pMaterials);
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

Matrix4x4 GLTFNode::CreateMatrix(const Vector3& scale, const Matrix4x4& rotate, const Vector3& translate)
{
	Matrix4x4 matrix(1.0);
	matrix *= glm::scale(matrix, scale);
	matrix *= rotate;
	matrix *= glm::translate(matrix, translate);
	return matrix;
}

void GLTFNode::UpdateMatrix(const Vector<int>& roots, Vector<GLTFNode>& nodes)
{
	for (int i = 0; i < roots.size(); i++) {
		UpdateMatrixRecursive(nodes, roots[i], Matrix4x4(1));
	}
}

void GLTFNode::UpdateMatrixRecursive(Vector<GLTFNode>& nodes, int index, const Matrix4x4& matrix)
{
	auto& node = nodes[index];
	node.SetMatrix(matrix * node.GetLocalMatrix());
	for (const auto& child : node.GetChild()) {
		UpdateMatrixRecursive(nodes, child, node.GetMatrix());
	}
}
Vector<GLTFNode::GpuObject> GLTFNode::CreateGpuObject(const Vector<GLTFNode>& nodes, const Vector<GLTFSkin>& skins)
{
	Vector<GLTFNode::GpuObject> bufferObject(nodes.size());
	for (size_t i = 0; i < nodes.size(); i++) {
		bufferObject[i].localMatrix = nodes[i].GetLocalMatrix();
		bufferObject[i].matrix = nodes[i].GetMatrix();
		if (nodes[i].GetSkinId() >= 0) {
			bufferObject[i].jointCount = skins[nodes[i].GetSkinId()].GetJointNodeIndex().size();
		} else {
			bufferObject[i].jointCount = -1;
		}
	}

	return bufferObject;
}


bool InTime(float begin, float end, float time)
{
	return begin < time && time < end;
}

void GLTFAnimation::Update(const Vector<GLTFAnimation>& animations, Vector<GLTFNode>& nodes, float time)
{
	for (const auto& animation : animations) {
		for (size_t i = 0; i < animation.GetChannels().size(); i++) {
			const auto& channel = animation.GetChannels()[i];
			const auto& sampler = animation.GetSamplers()[channel.sampler];
			auto translate = Vector4();
			auto scale = Vector3(1.0);
			auto rotate = Matrix4x4(1.0);
			for (size_t j = 0; j < sampler.timer.size() - 1; j++) {
				if (!InTime(sampler.timer[j], sampler.timer[j + 1], time)) { continue; }
				float u = std::max(0.0f, time - sampler.timer[j]) / (sampler.timer[j + 1] - sampler.timer[j]);
				if (u > 1.0f) continue;
				switch (channel.path) {
				case Channel::Path::Translate:
					translate = glm::mix(sampler.transform[j], sampler.transform[j + 1], u);
					break;
				case Channel::Path::Scale:
					scale = glm::mix(sampler.transform[j], sampler.transform[j + 1], u);
					break;
				case Channel::Path::Rotate:
					rotate = glm::mat4_cast(glm::normalize(glm::slerp(
						CreateQuart(sampler.transform[j]),
						CreateQuart(sampler.transform[j + 1]),
						u)));
					break;
				case Channel::Path::Weight:
					break;
				default:
					break;
				}
			}

			nodes[channel.node].SetLocalMatrix(GLTFNode::CreateMatrix(scale, rotate, translate));
		}
	}
}

void GLTFScene::UpdateData(float time)
{
	GLTFAnimation::Update(m_animation, m_nodes, time);
	UpdateMatrix();
}

void GLTFScene::UpdateMatrix()
{
	if (m_pMatrixGpuUpdater) {
		m_gpu.node = GLTFNode::CreateGpuObject(m_nodes, m_skins);
		m_pNodeBuffer->BufferSubData<GLTFNode::GpuObject>(0, m_gpu.node);
		m_pMatrixGpuUpdater->Execute(m_pNodeBuffer);
	} else {
		GLTFNode::UpdateMatrix(m_roots, m_nodes);
		m_gpu.node = GLTFNode::CreateGpuObject(m_nodes, m_skins);
		m_pNodeBuffer->BufferSubData<GLTFNode::GpuObject>(0, m_gpu.node);
	}
}
}
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
	if (m_pMaterials) {
		delete m_pMaterials;
		m_pMaterials = nullptr;
	}

	if (m_pMatrixGpuUpdater) {
		delete m_pMatrixGpuUpdater;
		m_pMatrixGpuUpdater = nullptr;
	}

	if (m_pNodeBuffer) {
		delete m_pNodeBuffer;
		m_pNodeBuffer = nullptr;
	}
}
void GLTFScene::Initialize()
{
	m_nodeBufferObject = CreateNodeBufferObject(m_nodes, m_skins);
	if (m_pNodeBuffer == nullptr) {
		m_pNodeBuffer = new GLBuffer();
		m_pNodeBuffer->Create(DATA_UNKNOWN, m_nodeBufferObject.size(), sizeof(GLTFNodeBufferObject), m_nodeBufferObject.data());
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
	m_pMaterials->Create(m_material.size(), sizeof(GLTFMaterial));
	m_pMaterials->BufferSubData(0, m_material.size(), sizeof(GLTFMaterial), m_material.data());
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

Matrix4x4 CreateMatrix(const Vector3& scale, const Matrix4x4& rotate, const Vector3& translate)
{
	Matrix4x4 matrix(1.0);
	matrix *= glm::scale(matrix, scale);
	matrix *= rotate;
	matrix *= glm::translate(matrix, translate);
	return matrix;
}

bool InTime(float begin, float end, float time)
{
	return begin < time && time < end;
}

void GLTFScene::UpdateMatrixRecursive(int index, const Matrix4x4& mat)
{
	auto& node = m_nodes[index];
	node.SetMatrix(mat * node.GetLocalMatrix());
	for (const auto& child : node.GetChild()) {
		UpdateMatrixRecursive(child, node.GetMatrix());
	}
}

Vector<GLTFNodeBufferObject> GLTFScene::CreateNodeBufferObject(const Vector<GLTFNode>& nodes, Vector<GLTFSkin>& skins)
{
	Vector<GLTFNodeBufferObject> bufferObject(nodes.size());
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

void GLTFScene::UpdateData(float time)
{
	for (const auto& animation : m_animation) {
		for (size_t i = 0; i < animation.GetChannels().size(); i++) {
			const auto& channel = animation.GetChannels()[i];
			const auto& sampler = animation.GetSamplers()[channel.sampler];
			Vector4 translate = Vector4();
			Vector3 scale = Vector3(1.0);
			Matrix4x4 rotate = Matrix4x4(1.0);
			for (size_t j = 0; j < sampler.timer.size() - 1; j++) {
				if (!InTime(sampler.timer[j], sampler.timer[j + 1], time)) { continue; }
				float u = std::max(0.0f, time - sampler.timer[j]) / (sampler.timer[j + 1] - sampler.timer[j]);
				if (u > 1.0f) continue;
				switch (channel.path) {
				case GLTFAnimation::Channel::Path::Translate:
					translate = glm::mix(sampler.transform[j], sampler.transform[j + 1], u);
					break;
				case GLTFAnimation::Channel::Path::Scale:
					scale = glm::mix(sampler.transform[j], sampler.transform[j + 1], u);
					break;
				case GLTFAnimation::Channel::Path::Rotate:
					rotate = glm::mat4_cast(glm::normalize(glm::slerp(
						CreateQuart(sampler.transform[j]),
						CreateQuart(sampler.transform[j + 1]),
						u)));
					break;
				case GLTFAnimation::Channel::Path::Weight:
					break;
				default:
					break;
				}
			}

			m_nodes[channel.node].SetLocalMatrix(CreateMatrix(scale, rotate, translate));
		}
	}
	UpdateMatrix();
}

void GLTFScene::UpdateMatrix()
{
	if (m_pMatrixGpuUpdater) {
		m_nodeBufferObject = CreateNodeBufferObject(m_nodes, m_skins);
		m_pNodeBuffer->BufferSubData(0, m_nodeBufferObject.size(), sizeof(GLTFNodeBufferObject), m_nodeBufferObject.data());
		m_pMatrixGpuUpdater->Execute(m_pNodeBuffer);
	} else {
		for (int i = 0; i < m_roots.size(); i++) {
 			UpdateMatrixRecursive(m_roots[i], Matrix4x4(1));
		}

		m_nodeBufferObject = CreateNodeBufferObject(m_nodes, m_skins);
		m_pNodeBuffer->BufferSubData(0, m_nodeBufferObject.size(), sizeof(GLTFNodeBufferObject), m_nodeBufferObject.data());
	}
}
}
#include "GLTFStruct.h"
#include "Texture.h"
#include "GLTFLoader.h"
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>
#include "GLTFShader.h"
#include "GLBuffer.h"
#include "GLUtility.h"
namespace KI
{

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
	m_pShader->SetMaterialBuffer(m_pMaterials);
	for (const auto& node : m_nodes) {
		m_pShader->SetModel(node.GetMatrix());
		if (node.GetMeshId() == -1) { continue; }
		const auto& mesh = m_meshes[node.GetMeshId()];
		if (mesh.GetBufferIndex() == -1) { continue; }
		const auto& meshBuffer = m_meshBuffer[mesh.GetBufferIndex()];
		m_pShader->SetVertexBuffer(meshBuffer.pVertex.get(), meshBuffer.format);
		m_pShader->SetIndexBuffer(meshBuffer.pIndex.get());
		for (const auto& primitive : mesh.GetPrimitives()) {
			m_pShader->BindBufferIndex(0, primitive.materialIndex);
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

void GLTFScene::UpdateMatrix(int index, const Matrix4x4& mat)
{
	auto& node = m_nodes[index];
	node.SetMatrix(mat * node.GetLocalMatrix());
	for (const auto& child : node.GetChild()) {
		UpdateMatrix(child, node.GetMatrix());
	}
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
			Printf(m_nodes[channel.node].GetLocalMatrix());
		}

		for (int i = 0; i < m_roots.size(); i++) {
			UpdateMatrix(m_roots[i], m_nodes[m_roots[i]].GetLocalMatrix());
		}
	}
}

}
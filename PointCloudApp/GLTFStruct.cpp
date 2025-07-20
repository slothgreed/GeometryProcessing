#include "GLTFStruct.h"
#include "Utility.h"
#include "Texture.h"
#include "GLTFLoader.h"
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>
#include "Utility.h"
namespace KI
{

GLTFSkin::GpuObject GLTFSkin::CreateGpuObject(const Vector<GLTFSkin>& skins)
{
	assert(skins.size() == 1);
	std::vector<float> memory(CalcGpuMemorySize(skins) /4);
	int offset = 0;
	for (const auto& skin : skins) {
		{
			float skeleton = skin.m_rootSkeleton;
			memcpy(&memory[offset], &skeleton, sizeof(float));
			offset += 1;
		}

		{
			float size = (float)skin.m_jointNodeIndex.size();
			memcpy(&memory[offset], &size, sizeof(float));
			offset += 1;
		}

		for (size_t i = 0; i < skin.m_jointNodeIndex.size(); i++) {
			{
				float joint = skin.m_jointNodeIndex[i];
				memcpy(&memory[offset], &joint, sizeof(float));
				offset += 1;
			}
	
			{
				memcpy(&memory[offset], &skin.m_inverseBindMatrix[i], sizeof(Matrix4x4));
				offset += 16;
			}

			{
				memcpy(&memory[offset], &skin.m_jointMatrix[i],sizeof(Matrix4x4));
				offset += 16;
			}

		}

	}

	GLTFSkin::GpuObject gpu;
	gpu.data = std::move(memory);
	return gpu;
}

int GLTFSkin::CalcGpuMemorySize(const Vector<GLTFSkin>& skins)
{
	int memorySize = 0;
	for (const auto& skin : skins) {
		memorySize += sizeof(float);	// rootIndex;
		memorySize += sizeof(float);	// jointSize;
		memorySize += skin.m_jointNodeIndex.size() * sizeof(float);
		memorySize += skin.m_inverseBindMatrix.size() * sizeof(Matrix4x4);
		memorySize += skin.m_jointMatrix.size() * sizeof(Matrix4x4);
	}

	return memorySize;
}
Matrix4x4 GLTFNode::CreateMatrix(const Vector3& scale, const Matrix4x4& rotate, const Vector3& translate)
{
	Matrix4x4 matrix(1.0);
	matrix = glm::translate(matrix, translate);
	matrix *= rotate;
	matrix = glm::scale(matrix, scale);
	return matrix;
}

const Matrix4x4& GLTFNode::GetLocalMatrix() const
{
	if (m_matrixType == Base) {
		return m_baseMatrix;
	} else {
		return m_trsMatrix;
	}
}
Matrix4x4 GLTFNode::CreateTRSMatrix() const
{
	return CreateMatrix(m_scale, m_rotate, m_translate);
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

void GLTFNode::SetScale(const Vector3& local)
{
	if (m_scale == local) { return; }
	m_scale = local; 
	m_trsMatrix = CreateTRSMatrix();
}
void GLTFNode::SetRotate(const Matrix4x4& local)
{
	if (m_rotate == local) { return; }
	m_rotate = local;
	m_trsMatrix = CreateTRSMatrix();
}
void GLTFNode::SetTranslate(const Vector3& local)
{
	if (m_translate == local) { return; }
	m_translate = local;
	m_trsMatrix = CreateTRSMatrix();
}
Vector<GLTFNode::GpuObject> GLTFNode::CreateGpuObject(const Vector<GLTFNode>& nodes, const Vector<GLTFSkin>& skins)
{
	Vector<GLTFNode::GpuObject> bufferObject(nodes.size());
	for (size_t i = 0; i < nodes.size(); i++) {
		bufferObject[i].scale = Vector4(nodes[i].GetScale(), 0.0f);
		bufferObject[i].translate = Vector4(nodes[i].GetTranslate(), 0.0f);
		bufferObject[i].rotate = nodes[i].GetRotate();
		bufferObject[i].localMatrix = nodes[i].GetLocalMatrix();
		bufferObject[i].matrix = nodes[i].GetMatrix();
		bufferObject[i].skinId = nodes[i].GetSkinId();
	}

	return bufferObject;
}

bool InTime(float begin, float end, float time)
{
	return begin < time && time <= end;
}

Vector<GLTFAnimation::SamplerGpuObject> GLTFAnimation::CreateSamplerGpuObject()
{
	Vector<GLTFAnimation::SamplerGpuObject> gpus;
	for (const auto& sampler : m_samplers) {
		assert(sampler.timer.size() == sampler.transform.size());
		GLTFAnimation::SamplerGpuObject gpu;
		gpu.data.resize(sampler.timer.size() * 5 + 1);
		gpu.data[0] = sampler.timer.size();
		for (size_t i = 0; i < sampler.timer.size(); i++) {
			gpu.data[5 * i + 1] = sampler.timer[i];
			gpu.data[5 * i + 2] = sampler.transform[i].x;
			gpu.data[5 * i + 3] = sampler.transform[i].y;
			gpu.data[5 * i + 4] = sampler.transform[i].z;
			gpu.data[5 * i + 5] = sampler.transform[i].w;
		}
		gpus.push_back(std::move(gpu));
	}

	return gpus;
}
Vector<GLTFAnimation::ChannelGpuObject> GLTFAnimation::CreateChannelGpuObject()
{
	return m_chennels;
}

void GLTFAnimation::Update(const Vector<GLTFAnimation>& animations, Vector<GLTFNode>& nodes, float time)
{
	for (const auto& animation : animations) {
		time = fmod(time, animation.GetMaxTime());
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
					nodes[channel.node].SetTranslate(
						glm::mix(sampler.transform[j], sampler.transform[j + 1], u));
					break;
				case Channel::Path::Scale:
					nodes[channel.node].SetScale(
						scale = glm::mix(sampler.transform[j], sampler.transform[j + 1], u));
					break;
				case Channel::Path::Rotate:
					nodes[channel.node].SetRotate(glm::mat4_cast(glm::normalize(glm::slerp(
						glmUtil::CreateQuart(sampler.transform[j]),
						glmUtil::CreateQuart(sampler.transform[j + 1]),
						u))));
					break;
				case Channel::Path::Weight:
					break;
				default:
					break;
				}
			}
		}
	}
}

void GLTFAnimation::CalcMaxTime()
{
	m_maxTime = 0;
	for (const auto& sampler : m_samplers) {
		m_maxTime = std::max(sampler.timer[sampler.timer.size() - 1],m_maxTime);
	}
}

void GLTFSkin::Update(Vector<GLTFSkin>& skins, const Vector<GLTFNode>& nodes)
{
	for (auto& node : nodes) {
		if (node.GetSkinId() < 0) continue;
		auto& skin = skins[node.GetSkinId()];
		auto invMat = glm::inverse(node.GetMatrix());
		for (size_t i = 0; i < skin.m_jointNodeIndex.size(); i++) {
			const auto& jointNode = nodes[skin.m_jointNodeIndex[i]];
			skin.m_jointMatrix[i] = invMat * jointNode.GetMatrix() * skin.m_inverseBindMatrix[i];
		}
	}
}
}

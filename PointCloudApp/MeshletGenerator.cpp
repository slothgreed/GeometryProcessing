#include "MeshletGenerator.h"
#include "Utility.h"
#include <algorithm>

namespace KI
{
ShaderPath MeshletShader::GetShaderPath()
{
	ShaderPath path;

	path.shader[SHADER_PROGRAM_MESH] = "meshlet.mesh";
	path.shader[SHADER_PROGRAM_FRAG] = "meshlet.frag";

	return path;
}

void MeshletShader::GetUniformLocation()
{
	m_uniform[UNIFORM::MODEL] = glGetUniformLocation(Handle(), "u_Model");
}

void MeshletShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
	OUTPUT_GLERROR;
}

void MeshletShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(m_uniform[UNIFORM::MODEL], 1, GL_FALSE, &value[0][0]);
}

void MeshletShader::SetPosition(GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pBuffer->Handle());
	OUTPUT_GLERROR;
}

void MeshletShader::SetMeshlet(GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pBuffer->Handle());
	OUTPUT_GLERROR;
}

void MeshletShader::SetIndex(GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pBuffer->Handle());
	OUTPUT_GLERROR;
}

void MeshletShader::Draw(int first, int count)
{
	glDrawMeshTasksNV(first, count);
	OUTPUT_GLERROR;
}
MeshletGenerator::MeshletGenerator()
{
}

MeshletGenerator::~MeshletGenerator()
{
}

Meshlet MeshletGenerator::Execute(const HalfEdgeStruct& halfEdge, int loopNum)
{
	return ExecuteBinaryFetch(halfEdge, loopNum);
}
Meshlet MeshletGenerator::ExecuteBinaryFetch(const HalfEdgeStruct& halfEdge, int loopNum)
{
	struct Cluster
	{
		Cluster(int _index = -1){
			index = _index;
			color = ColorUtility::CreateRandom4();
		}
		Vector4 color;
		int index;
		Vector<unsigned int> face;
		Vector3 CalcGravity(const HalfEdgeStruct& halfEdge)
		{
			Vector3 gravity = Vector3(0.0f);
			for (auto index : face) {
				gravity += halfEdge.CalcGravity(halfEdge.GetFace(index));
			}

			return gravity;
		}

		USet<Cluster*> CalcAroundCluster(const Vector<Cluster*>& faceToCluster, const HalfEdgeStruct& halfEdge)
		{
			USet<Cluster*> ret;
			for (auto faceIndex : face) {
				auto aroundFace = halfEdge.GetAroundFace(halfEdge.GetFace(faceIndex));
				for (auto around : aroundFace) {
					auto cluster = faceToCluster[around];
					// マージ済みでなく自身でない場合
					if (cluster->IsActive() && cluster->index != faceToCluster[faceIndex]->index) {
						ret.insert(cluster);
					}
				}
			}
			return ret;
		}


		USet<int> CalcAroundCluster(const Vector<Cluster>& clusters, const Vector<Cluster*>& faceToCluster, const HalfEdgeStruct& halfEdge)
		{
			USet<int> ret;
			for (auto faceIndex : face) {
				auto aroundFace = halfEdge.GetAroundFace(halfEdge.GetFace(faceIndex));
				for (auto around : aroundFace) {
					auto cluster = faceToCluster[around];
					// マージ済みでなく自身でない場合
					if (cluster->IsActive() && cluster->index != faceToCluster[faceIndex]->index) {
						ret.insert(cluster->index);
					}
				}
			}
			return ret;
		}

		void Merge(Vector<Cluster*>& faceToCluster, Cluster& rhs)
		{
			for (auto faceIndex : rhs.face) {
				faceToCluster[faceIndex] = this;
			}
			face.insert(face.end(),
				std::make_move_iterator(rhs.face.begin()),
				std::make_move_iterator(rhs.face.end()));
			rhs.face.clear();
		}
		bool IsActive() const { return face.size() != 0; }
		bool operator<(const Cluster& cluster) const { return face.size() < cluster.face.size(); }
	};

	size_t clusterNum = halfEdge.GetFaceNum();
	Vector<Cluster> clusters(clusterNum);
	Vector<Cluster*> faceToCluster(halfEdge.GetFaceNum());
	for (size_t i = 0; i < clusters.size(); i++) {
		clusters[i].index = i;
		clusters[i].face.push_back(i);
		faceToCluster[i] = &clusters[i];
	}

	int clusterMaxNum = 20;
	for (int j = 0; j < loopNum; j++) {
		for (size_t i = 0; i < clusters.size(); i++) {
			if (!clusters[i].IsActive() || clusters[i].face.size() == clusterMaxNum) { continue; }
			float minNum = std::numeric_limits<float>::infinity();
			Cluster* minArond = nullptr;
			auto aroundCluster = clusters[i].CalcAroundCluster(clusters, faceToCluster, halfEdge);
			for (auto around : aroundCluster) {
				auto faceNum = clusters[around].face.size();
				if (faceNum + clusters[i].face.size() > clusterMaxNum) {
					continue;
				}

				if (faceNum < minNum) {
					minNum = faceNum;
					minArond = &clusters[around];
				}
			}

			if (minArond) {
				clusters[i].Merge(faceToCluster, *minArond);
			}
		}
	}
	Vector<Vector4> colors(halfEdge.GetFaceNum());
	for (size_t i = 0; i < colors.size(); i++) {
		colors[i] = faceToCluster[i]->color;
	}

	Meshlet meshlet;
	int offset = 0;
	int index = 0;
	meshlet.color = std::move(colors);
	for (const auto& cluster : clusters) {
		if (cluster.face.size() == 0) { continue; }
		Meshlet::Cluster data;
		data.x = offset;
		data.y = cluster.face.size() * 3;
		data.z = index++; data.w = 0;
		offset += data.y;

		meshlet.data.push_back(std::move(data));
		meshlet.maxVertex = std::max(meshlet.maxVertex, cluster.face.size() * 3);
		for (auto face : cluster.face) {
			const auto& faceIndex = halfEdge.GetFace(face);
			meshlet.index.push_back(faceIndex.position[0]);
			meshlet.index.push_back(faceIndex.position[1]);
			meshlet.index.push_back(faceIndex.position[2]);
		}
	}

	return meshlet;
}
}
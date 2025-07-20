#include "MeshletGenerator.h"
#include "Utility.h"
#include <algorithm>

namespace KI
{
ShaderPath MeshletShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("meshlet.h");
	path.header.push_back("common.h");

	path.extension[SHADER_PROGRAM_TASK].push_back("#extension GL_NV_mesh_shader : require\n");
	path.shader[SHADER_PROGRAM_TASK] = "meshlet.task";
	path.extension[SHADER_PROGRAM_MESH].push_back("#extension GL_NV_mesh_shader : enable\n");
	path.shader[SHADER_PROGRAM_MESH] = "meshlet.mesh";
	path.extension[SHADER_PROGRAM_FRAG].push_back("#extension GL_NV_fragment_shader_barycentric : enable\n");
	path.shader[SHADER_PROGRAM_FRAG] = "meshlet.frag";

	return path;
}

void MeshletShader::FetchUniformLocation()
{
	u_Model = GetUniformLocation("u_Model");
	u_NormalMatrix = GetUniformLocation("u_Normal");
	u_MeshletNum = GetUniformLocation("u_MeshletNum");
	u_CullSize = GetUniformLocation("u_CullSize");
}

void MeshletShader::SetCamera(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->Handle());
	OUTPUT_GLERROR;
}

void MeshletShader::SetModel(const Matrix4x4& value)
{
	glUniformMatrix4fv(u_Model, 1, GL_FALSE, &value[0][0]);
}

void MeshletShader::SetNormalMatrix(const Matrix3x3& value)
{
	glUniformMatrix3fv(u_NormalMatrix, 1, GL_FALSE, &value[0][0]);
}

void MeshletShader::SetMeshletNum(int meshletNum)
{
	BindUniform(u_MeshletNum, meshletNum);
}


void MeshletShader::SetCullSize(int cullSize)
{
	BindUniform(u_CullSize, cullSize);
	OUTPUT_GLERROR;
}
void MeshletShader::SetTaskToMeshNum(const GLBuffer* pBuffer)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pBuffer->Handle());
	OUTPUT_GLERROR;
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
				gravity += halfEdge.CalcGravity(halfEdge.GetIndexedFace(index));
			}

			return gravity;
		}

		USet<Cluster*> CalcAroundCluster(const Vector<Cluster*>& faceToCluster, const HalfEdgeStruct& halfEdge)
		{
			USet<Cluster*> ret;
			for (auto faceIndex : face) {
				auto aroundFace = halfEdge.GetAroundFace(halfEdge.GetIndexedFace(faceIndex));
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
				auto aroundFace = halfEdge.GetAroundFace(halfEdge.GetIndexedFace(faceIndex));
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

	int maxTriangle = GLAPIExt::Info()->GetMeshletMaxPrimitive();
	int maxVertex = GLAPIExt::Info()->GetMeshletMaxVertex();
	int clusterMaxNum = std::min(maxTriangle, maxVertex / 3);

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

	Meshlet meshlet;
	int offset = 0;
	int index = 0;
	for (const auto& cluster : clusters) {
		if (cluster.face.size() == 0) { continue; }
		Meshlet::Cluster data;
		data.offset = offset;
		data.size = cluster.face.size() * 3;
		data.meshletIndex = index++; data.padding = 0;
		offset += data.size;

		meshlet.maxVertex = std::max(meshlet.maxVertex, cluster.face.size() * 3);
		auto normal = Vector3(0);
		BDB box;
		for (auto face : cluster.face) {
			const auto& faceIndex = halfEdge.GetIndexedFace(face);
			const auto& facePos = halfEdge.GetFace(face);
			meshlet.index.push_back(faceIndex.position[0]);
			meshlet.index.push_back(faceIndex.position[1]);
			meshlet.index.push_back(faceIndex.position[2]);
			normal += halfEdge.CalcFaceNormal(face);
			box.Add(facePos.pos0);	box.Add(facePos.pos1);	box.Add(facePos.pos2);
		}
		normal /= cluster.face.size();
		data.normal = Vector4(glm::normalize(normal), 1.0);
		data.boxMin = Vector4(box.Min(), 1.0);
		data.boxMax = Vector4(box.Max(), 1.0);
		meshlet.cluster.push_back(std::move(data));
	}

	return meshlet;
}





MeshletProfiler::MeshletProfiler()
{
	glGenQueries(1, &m_primitiveQuery);
	OUTPUT_GLERROR;
	glGenQueries(1, &m_vertexQuery);
	OUTPUT_GLERROR;
}

MeshletProfiler::~MeshletProfiler()
{
	glDeleteQueries(1, &m_primitiveQuery);
	OUTPUT_GLERROR;
	glDeleteQueries(1, &m_vertexQuery);
	OUTPUT_GLERROR;
}

void MeshletProfiler::BeginQuery()
{
	glBeginQuery(GL_PRIMITIVES_GENERATED, m_primitiveQuery);
	OUTPUT_GLERROR;
	glBeginQuery(GL_VERTICES_SUBMITTED, m_vertexQuery);
	OUTPUT_GLERROR;
}

void MeshletProfiler::EndQuery()
{
	glEndQuery(GL_PRIMITIVES_GENERATED);
	OUTPUT_GLERROR;
	glEndQuery(GL_VERTICES_SUBMITTED);
	OUTPUT_GLERROR;

	glGetQueryObjectui64v(m_primitiveQuery, GL_QUERY_RESULT, &m_primitives);
	OUTPUT_GLERROR;
	glGetQueryObjectui64v(m_vertexQuery, GL_QUERY_RESULT, &m_vertices);
	OUTPUT_GLERROR;
}

void MeshletProfiler::ShowUI()
{
	ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f, ImGui::GetIO().DisplaySize.y - 10.0f);
	ImVec2 window_pos_pivot = ImVec2(1.0f, 1.0f); // 右下
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.4f); // 半透明

	ImGui::Begin("Meshlet Profiler", nullptr,
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav);

	ImGui::Text("Meshlet Profiling:");
	ImGui::Separator();
	ImGui::Text("Primitives : %llu", m_primitives);
	ImGui::Text("Vertices   : %llu", m_vertices);

	ImGui::End();
}

}
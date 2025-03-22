#include "Voxelizer.h"
#include "HalfEdgeNode.h"
#include "BDB.h"
//#define DEBUG_VOXEL

namespace KI
{

Vector3i ToResolute3i(int resolution)
{
	return Vector3i(UIntBool::Size(resolution), resolution, resolution);
}
int To1ArraySize(int resolution)
{
	return UIntBool::Size(resolution) * resolution * resolution;
}
Voxelizer::Voxelizer(HalfEdgeNode* pNode)
	: m_pNode(pNode)
	, m_resolution(256)
	, m_pCpuVoxel(new GLBuffer())
	, m_pVec4PointBuffer(new GLBuffer())
{
	m_gpuVoxelizer.Build();
}

Voxelizer::~Voxelizer()
{
	delete m_pCpuVoxel;
}

vec3 getVoxel(uint location, int bitIndex)
{
	int index = int(location) * 32 + bitIndex;
	int z = index / (32 * 32);
	int y = (index / 32) % 32;
	int x = index % 32;
	return vec3(x, y, z);
}


void Voxelizer::Execute(GLBuffer* pointBuffer, GLBuffer* indexBuffer)
{
	if (m_pVec4PointBuffer->Num() == 0) {
		if (pointBuffer->ComponentSize() == 3) {
			Vector<Vector3> point(pointBuffer->Num());
			pointBuffer->GetBufferData(point);
			auto vec4 = TypeConverter::Convert4f(point);
			m_pVec4PointBuffer->Create(vec4);
		}
	}

	auto resultGPU = m_gpuVoxelizer.Execute(m_pNode->GetBoundBox(), m_resolution, m_pVec4PointBuffer, indexBuffer, m_pNode->GetData()->GetFaceNum());

#ifdef DEBUG_VOXEL
	// CPU Debug
	{
		auto indexs = m_pNode->GetData()->CreateIndexBufferData();
		std::vector<unsigned int> resultCPU(To1ArraySize(m_resolution));

		for (int i = 0; i < m_pNode->GetData()->GetFaceNum(); i++) {
			ExecuteCPU(m_pNode->GetBoundBox(), i, m_pNode->GetData()->GetPosition(), indexs, resultCPU);
		}

		m_pCpuVoxel->Create(resultCPU);
	
		for (int i = 0; i < resultCPU.size(); i++) {
			if (resultCPU[i] != resultGPU[i]) {
				int a = 0;
			}
		}
	}
#endif // DEBUG_VOXEL
}

void Voxelizer::ExecuteCPU(const BDB& bdb, int triIdx, const std::vector<Vector3>& position, const std::vector<unsigned int>& indexs, std::vector<unsigned int>& result)
{
	if(triIdx >= m_pNode->GetData()->GetFaceNum()) {return;}
	
	float u_pitch = bdb.MaxLength() / m_resolution;
	vec3 p0 = position[indexs[3 * triIdx + 0]] - bdb.Min();
	vec3 p1 = position[indexs[3 * triIdx + 1]] - bdb.Min();
	vec3 p2 = position[indexs[3 * triIdx + 2]] - bdb.Min();
	vec3 edge0 = p1 - p0;
	vec3 edge1 = p2 - p1;
	vec3 edge2 = p0 - p2;
	
	vec3 normal = normalize(cross(edge0,edge1));
	vec3 c = vec3(0);
	if(normal.x > 0.0){c.x = u_pitch;}
	if(normal.y > 0.0){c.y = u_pitch;}
	if(normal.z > 0.0){c.z = u_pitch;}
	float d1 = dot(normal,(c - p0));
	float d2 = dot(normal,(vec3(u_pitch) - c) - p0);
	vec3 bdbMin_ = min(min(p0,p1),p2);
	vec3 bdbMax_ = max(max(p0,p1),p2);
	ivec3 bdbMin = clamp(ivec3(bdbMin_ / u_pitch), ivec3(0),ivec3(m_resolution - 1));
	ivec3 bdbMax = clamp(ivec3(bdbMax_ / u_pitch), ivec3(0),ivec3(m_resolution - 1));

	vec2 xyEdge[3];
	float xyDiff[3];
	xyEdge[0] = vec2(-edge0.y, edge0.x);
	xyEdge[1] = vec2(-edge1.y, edge1.x);
	xyEdge[2] = vec2(-edge2.y, edge2.x);
	if(normal.z < 0){xyEdge[0] = -xyEdge[0]; xyEdge[1] = -xyEdge[1]; xyEdge[2] = -xyEdge[2];}
	xyDiff[0] = (-dot(xyEdge[0], vec2(p0.x, p0.y))) + max(0.0f, u_pitch * xyEdge[0].x) + max(0.0f, u_pitch * xyEdge[0].y);
	xyDiff[1] = (-dot(xyEdge[1], vec2(p1.x, p1.y))) + max(0.0f, u_pitch * xyEdge[1].x) + max(0.0f, u_pitch * xyEdge[1].y);
	xyDiff[2] = (-dot(xyEdge[2], vec2(p2.x, p2.y))) + max(0.0f, u_pitch * xyEdge[2].x) + max(0.0f, u_pitch * xyEdge[2].y);
	
	vec2 yzEdge[3];
	float yzDiff[3];
	yzEdge[0] = vec2(-edge0.z, edge0.y);
	yzEdge[1] = vec2(-edge1.z, edge1.y);
	yzEdge[2] = vec2(-edge2.z, edge2.y);
	if(normal.z < 0){yzEdge[0] = -yzEdge[0]; yzEdge[1] = -yzEdge[1]; yzEdge[2] = -yzEdge[2];}
	yzDiff[0] = (-dot(yzEdge[0], vec2(p0.y, p0.z))) + max(0.0f, u_pitch * yzEdge[0].x) + max(0.0f, u_pitch * yzEdge[0].y);
	yzDiff[1] = (-dot(yzEdge[1], vec2(p1.y, p1.z))) + max(0.0f, u_pitch * yzEdge[1].x) + max(0.0f, u_pitch * yzEdge[1].y);
	yzDiff[2] = (-dot(yzEdge[2], vec2(p2.y, p2.z))) + max(0.0f, u_pitch * yzEdge[2].x) + max(0.0f, u_pitch * yzEdge[2].y);
	
	vec2 zxEdge[3];
	float zxDiff[3];
	zxEdge[0] = vec2(-edge0.x, edge0.z);
	zxEdge[1] = vec2(-edge1.x, edge1.z);
	zxEdge[2] = vec2(-edge2.x, edge2.z);
	if(normal.z < 0){zxEdge[0] = -zxEdge[0]; yzEdge[1] = -zxEdge[1]; zxEdge[2] = -zxEdge[2];}
	zxDiff[0] = (-dot(zxEdge[0], vec2(p0.z, p0.x))) + max(0.0f, u_pitch * zxEdge[0].x) + max(0.0f, u_pitch * zxEdge[0].y);
	zxDiff[1] = (-dot(zxEdge[1], vec2(p1.z, p1.x))) + max(0.0f, u_pitch * zxEdge[1].x) + max(0.0f, u_pitch * zxEdge[1].y);
	zxDiff[2] = (-dot(zxEdge[2], vec2(p2.z, p2.x))) + max(0.0f, u_pitch * zxEdge[2].x) + max(0.0f, u_pitch * zxEdge[2].y);
	
	for(int i = bdbMin.x; i <= bdbMax.x; i++){
	for(int j = bdbMin.y; j <= bdbMax.y; j++){
	for(int k = bdbMin.z; k <= bdbMax.z; k++){

		//vec3 pos = vec3(i,j,k) * u_pitch;
		//float NdotP = dot(normal,pos);
		//if(((NdotP + d1) * (NdotP + d2) < 0.0f)){continue;}
		//
		//vec2 xyP = vec2(pos.x,pos.y);
		//if(dot(xyEdge[0],xyP) + xyDiff[0] < 0.0f){continue;}
		//if(dot(xyEdge[1],xyP) + xyDiff[1] < 0.0f){continue;}
		//if(dot(xyEdge[2],xyP) + xyDiff[2] < 0.0f){continue;}
		//
		//vec2 yzP = vec2(pos.y,pos.z);
		//if(dot(yzEdge[0],yzP) + yzDiff[0] < 0.0f){continue;}
		//if(dot(yzEdge[1],yzP) + yzDiff[1] < 0.0f){continue;}
		//if(dot(yzEdge[2],yzP) + yzDiff[2] < 0.0f){continue;}
		//
		//
		//vec2 zxP = vec2(pos.x,pos.y);
		//if(dot(zxEdge[0],zxP) + zxDiff[0] < 0.0f){continue;}
		//if(dot(zxEdge[1],zxP) + zxDiff[1] < 0.0f){continue;}
		//if(dot(zxEdge[2],zxP) + zxDiff[2] < 0.0f){continue;}
		
		uint location = i + j * m_resolution + k * m_resolution * m_resolution;
		uint location2 = location / 32;
		uint bit = location % 32;
		uint mask = 1 << bit | 0;
		result[location2] |= mask;

		auto v = getVoxel(location2, bit);
		if (v.x == i && v.y == j && v.z == k) {
		} else {
			int a = 0;
		}
	}}}
}
void Voxelizer::ShowUI(UIContext& ui)
{

}

void Voxelizer::ComputeShader::FetchUniformLocation()
{
	m_uniform[UNIFORM::MIN] = GetUniformLocation("u_min");
	m_uniform[UNIFORM::PITCH] = GetUniformLocation("u_pitch");
	m_uniform[UNIFORM::RESOLUTE] = GetUniformLocation("u_resolute");
	m_uniform[UNIFORM::TRIANGLENUM] = GetUniformLocation("u_triNum");
}

std::vector<unsigned int> Voxelizer::ComputeShader::Execute(const BDB& bdb, int resolution, GLBuffer* pointBuffer, GLBuffer* indexBuffer, int triangleNum)
{
	m_pVoxelBuffer->Create(To1ArraySize(resolution), sizeof(unsigned int));
	m_pVoxelBuffer->SetData(0);
	Use();
	BindUniform(m_uniform[UNIFORM::MIN], bdb.Min());
	BindUniform(m_uniform[UNIFORM::PITCH], bdb.MaxLength() / resolution);
	BindUniform(m_uniform[UNIFORM::RESOLUTE], resolution);
	BindUniform(m_uniform[UNIFORM::TRIANGLENUM], triangleNum);
	BindShaderStorage(0, pointBuffer->Handle());
	BindShaderStorage(1, indexBuffer->Handle());
	BindShaderStorage(2, m_pVoxelBuffer->Handle());
	Dispatch(IComputeShader::GetDispatchNum1D(Vector3i(1, 1, 1), triangleNum));

	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
#ifdef DEBUG_VOXEL
	std::vector<unsigned int> result(m_pVoxelBuffer->Num());
	m_pVoxelBuffer->GetBufferData(result.data(), result.size() * sizeof(unsigned int));
	return result;
#else
	return std::vector<unsigned int>();
#endif

}

void Voxelizer::Draw(GLBuffer* pointBuffer, GLBuffer* indexBuffer, int camera)
{
	if (!m_drawer.IsActive()) { m_drawer.Build(); }
	//if (!m_gpuVoxelizer.m_pVoxelBuffer) { return; }

	//Execute(pointBuffer, indexBuffer);
	m_drawer.Draw(camera, m_pNode->GetMatrix(), m_pNode->GetBoundBox(), m_resolution, m_gpuVoxelizer.m_pVoxelBuffer);
	//m_drawer.Draw(camera, m_pNode->GetMatrix(), m_pNode->GetBoundBox(), m_resolution, m_pCpuVoxel);
}

Voxelizer::ComputeShader::ComputeShader()
	: m_pVoxelBuffer(new GLBuffer())
{

}

Voxelizer::ComputeShader::~ComputeShader()
{
	delete m_pVoxelBuffer;
}

ShaderPath Voxelizer::ComputeShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.extension[SHADER_PROGRAM_COMPUTE].push_back("#extension GL_NV_gpu_shader5 : enable");
	path.shader[SHADER_PROGRAM_COMPUTE] = "algorithm/voxelizer.comp";
	return path;
}

void Voxelizer::MeshShader::FetchUniformLocation()
{
	m_uniform[UNIFORM::PITCH] = GetUniformLocation("u_pitch");
	m_uniform[UNIFORM::MIN] = GetUniformLocation("u_min");
	m_uniform[UNIFORM::MODEL] = GetUniformLocation("u_Model");
	m_uniform[UNIFORM::RESOLUTE] = GetUniformLocation("u_resolute");
	m_uniform[UNIFORM::ARRAYSIZE] = GetUniformLocation("u_arraySize");
}
ShaderPath Voxelizer::MeshShader::GetShaderPath()
{
	ShaderPath path;
	path.version = "version.h";
	path.header.push_back("common.h");
	path.shader[SHADER_PROGRAM_MESH] = "algorithm/voxelizer.mesh";
	path.shader[SHADER_PROGRAM_FRAG] = "algorithm/voxelizer.frag";
	path.extension[SHADER_PROGRAM_MESH].push_back("#extension GL_NV_mesh_shader : require\n");

	return path;
}

void Voxelizer::MeshShader::Draw(int camera, const Matrix4x4& matrix, const BDB& bdb, int resolution, GLBuffer* voxelBuffer)
{

	Use();
	BindUniform(m_uniform[UNIFORM::RESOLUTE], resolution);
	BindUniform(m_uniform[UNIFORM::PITCH], bdb.MaxLength() / resolution);
	BindUniform(m_uniform[UNIFORM::MIN], bdb.Min());
	BindUniform(m_uniform[UNIFORM::MODEL], matrix);
	BindUniform(m_uniform[UNIFORM::ARRAYSIZE], voxelBuffer->Num());
	BindShaderStorage(0, camera);
	BindShaderStorage(1, voxelBuffer->Handle());
	DrawMeshTasks(0, voxelBuffer->Num());
	UnUse();
}

}
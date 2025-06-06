#include "Voxelizer.h"
#include "HalfEdgeNode.h"
#include "BDB.h"
#include "PrimitiveNode.h"
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
	, m_resolution(32)
	, m_pCpuVoxel(new GLBuffer())
	, m_pVec4PointBuffer(new GLBuffer())
{
	m_gpuVoxelizer.Build();
}

Voxelizer::~Voxelizer()
{
	delete m_pVec4PointBuffer;
	delete m_pCpuVoxel;
}

ivec3 getVoxel(uint location, int bitIndex)
{
	int index = int(location) * 32 + bitIndex;
	int z = index / (32 * 32);
	int y = (index / 32) % 32;
	int x = index % 32;
	return ivec3(x, y, z);
}

bool IsBoundary(const std::vector<unsigned int>& voxel, int resolution, const ivec3& index)
{
	uint location = index.x + index.y * resolution + index.z * resolution * resolution;
	uint location2 = location / 32;
	uint bit = location % 32;
	uint mask = 1u << bit;
	return voxel[location2] & mask;
}

void SetBoundary(std::vector<unsigned int>& voxel, int resolution, const ivec3& index)
{
	uint location = index.x + index.y * resolution + index.z * resolution * resolution;
	uint location2 = location / 32;
	uint bit = location % 32;
	uint mask = 1u << bit;
	voxel[location2] |= mask;
}

std::vector<unsigned int> Create1DArray(int resolution)
{
	return std::vector<unsigned int>(To1ArraySize(resolution));
}


void Voxelizer::Execute(int resolute)
{
	m_resolution = resolute;
	if (m_pVec4PointBuffer->Num() == 0) {
		if (m_pNode->GetPositionGpu()->ComponentSize() == 3) {
			Vector<Vector3> point(m_pNode->GetPositionGpu()->Num());
			m_pNode->GetPositionGpu()->GetBufferData(point);
			auto vec4 = TypeConverter::Convert4f(point);
			m_pVec4PointBuffer->Create(vec4);
		}
	}

	
#ifdef DEBUG_VOXEL
	// CPU Debug
	{
		auto indexs = m_pNode->GetData()->CreateIndexBufferData();
		auto resultCPU = Create1DArray(m_resolution);
		for (int i = 0; i < m_pNode->GetData()->GetFaceNum(); i++) {
			ExecuteCPU(m_pNode->GetBoundBox(), i, m_pNode->GetData()->GetVertex(), indexs, resultCPU);
		}

		m_pCpuVoxel->Create(resultCPU);
	}
#else
	auto resultGPU = m_gpuVoxelizer.Execute(m_pNode->GetBoundBox(), m_resolution, m_pVec4PointBuffer, m_pNode->GetFaceIndexGpu(), m_pNode->GetData()->GetFaceNum());
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
	if (normal.z < 0) { xyEdge[0] = -xyEdge[0]; xyEdge[1] = -xyEdge[1]; xyEdge[2] = -xyEdge[2]; }
	xyDiff[0] = (-dot(xyEdge[0], vec2(p0.x, p0.y))) + max(0.0f, u_pitch * xyEdge[0].x) + max(0.0f, u_pitch * xyEdge[0].y);
	xyDiff[1] = (-dot(xyEdge[1], vec2(p1.x, p1.y))) + max(0.0f, u_pitch * xyEdge[1].x) + max(0.0f, u_pitch * xyEdge[1].y);
	xyDiff[2] = (-dot(xyEdge[2], vec2(p2.x, p2.y))) + max(0.0f, u_pitch * xyEdge[2].x) + max(0.0f, u_pitch * xyEdge[2].y);
	
	vec2 yzEdge[3];
	float yzDiff[3];
	yzEdge[0] = vec2(-edge0.z, edge0.y);
	yzEdge[1] = vec2(-edge1.z, edge1.y);
	yzEdge[2] = vec2(-edge2.z, edge2.y);
	if (normal.x < 0) { yzEdge[0] = -yzEdge[0]; yzEdge[1] = -yzEdge[1]; yzEdge[2] = -yzEdge[2]; }
	yzDiff[0] = (-dot(yzEdge[0], vec2(p0.y, p0.z))) + max(0.0f, u_pitch * yzEdge[0].x) + max(0.0f, u_pitch * yzEdge[0].y);
	yzDiff[1] = (-dot(yzEdge[1], vec2(p1.y, p1.z))) + max(0.0f, u_pitch * yzEdge[1].x) + max(0.0f, u_pitch * yzEdge[1].y);
	yzDiff[2] = (-dot(yzEdge[2], vec2(p2.y, p2.z))) + max(0.0f, u_pitch * yzEdge[2].x) + max(0.0f, u_pitch * yzEdge[2].y);
	
	vec2 zxEdge[3];
	float zxDiff[3];
	zxEdge[0] = vec2(-edge0.x, edge0.z);
	zxEdge[1] = vec2(-edge1.x, edge1.z);
	zxEdge[2] = vec2(-edge2.x, edge2.z);
	if(normal.y < 0){zxEdge[0] = -zxEdge[0]; zxEdge[1] = -zxEdge[1]; zxEdge[2] = -zxEdge[2];}
	zxDiff[0] = (-dot(zxEdge[0], vec2(p0.z, p0.x))) + max(0.0f, u_pitch * zxEdge[0].x) + max(0.0f, u_pitch * zxEdge[0].y);
	zxDiff[1] = (-dot(zxEdge[1], vec2(p1.z, p1.x))) + max(0.0f, u_pitch * zxEdge[1].x) + max(0.0f, u_pitch * zxEdge[1].y);
	zxDiff[2] = (-dot(zxEdge[2], vec2(p2.z, p2.x))) + max(0.0f, u_pitch * zxEdge[2].x) + max(0.0f, u_pitch * zxEdge[2].y);
	
	for(int i = bdbMin.x; i <= bdbMax.x; i++){
	for(int j = bdbMin.y; j <= bdbMax.y; j++){
	for(int k = bdbMin.z; k <= bdbMax.z; k++){

		vec3 pos = vec3(i,j,k) * u_pitch;
		float NdotP = dot(normal, pos);
		if (((NdotP + d1) * (NdotP + d2) > 0.0f)) { continue; }
		
		vec2 xyP = vec2(pos.x,pos.y);
		if (dot(xyEdge[0], xyP) + xyDiff[0] < 0.0f) { continue; }
		if (dot(xyEdge[1], xyP) + xyDiff[1] < 0.0f) { continue; }
		if (dot(xyEdge[2], xyP) + xyDiff[2] < 0.0f) { continue; }

		vec2 yzP = vec2(pos.y, pos.z);
		if (dot(yzEdge[0], yzP) + yzDiff[0] < 0.0f) { continue; }
		if (dot(yzEdge[1], yzP) + yzDiff[1] < 0.0f) { continue; }
		if (dot(yzEdge[2], yzP) + yzDiff[2] < 0.0f) { continue; }


		vec2 zxP = vec2(pos.z, pos.x);
		if (dot(zxEdge[0], zxP) + zxDiff[0] < 0.0f) { continue; }
		if (dot(zxEdge[1], zxP) + zxDiff[1] < 0.0f) { continue; }
		if (dot(zxEdge[2], zxP) + zxDiff[2] < 0.0f) { continue; }
		SetBoundary(result, m_resolution, ivec3(i, j, k));

	}}}
}

const char* const* Voxelizer::GetLabelString()
{
	static const char* parameter[] = {
		"Unknown",
		"INNER",
		"BORDER",
		"OUTER",
	};

	return parameter;
}
void Voxelizer::ShowUI(UIContext& ui)
{
	bool createLabelPoint = false;
	if (ImGui::Checkbox("VisibleLabel", &m_ui.visibleInOut)) {
		if (!m_ui.visibleInOut) {
			m_pNode->RemoveNode("Voxelizer::Label");
		} else {
			createLabelPoint = true;
		}
	}
	if (ImGui::Combo("CreateLabel", &m_ui.label, Voxelizer::GetLabelString(), static_cast<int>(Voxelizer::Label::NUM))) {
		createLabelPoint = true;
	}
	if (createLabelPoint) {
		
		std::shared_ptr<Primitive> pLabel = std::make_shared<Primitive>();
		Vector<Vector3> position;
		Vector<Vector3> color;
		CreateLabelPoint(position, color,(Voxelizer::Label)m_ui.label);
		pLabel->SetPosition(std::move(position));
		pLabel->SetColor(std::move(color));
		auto pNode = std::make_shared<PrimitiveNode>("Voxelizer::Label", pLabel);
		pNode->SetMatrix(m_pNode->GetMatrix());
		m_pNode->AddNode(pNode);
	}

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
	Dispatch(GetDispatchNum1D(triangleNum));

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	UnUse();
	glFinish();
	OUTPUT_GLERROR;
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
#ifdef DEBUG_VOXEL
	m_drawer.Draw(camera, m_pNode->GetMatrix(), m_pNode->GetBoundBox(), m_resolution, m_pCpuVoxel);
#else
	m_drawer.Draw(camera, m_pNode->GetMatrix(), m_pNode->GetBoundBox(), m_resolution, m_gpuVoxelizer.m_pVoxelBuffer);
#endif DEBUG_VOXEL
	
}

std::vector<glm::ivec3> Create26Neighbor()
{
	std::vector<glm::ivec3> offsets;
	for (int dz = -1; dz <= 1; ++dz)
	for (int dy = -1; dy <= 1; ++dy)
	for (int dx = -1; dx <= 1; ++dx) {
		if (dx == 0 && dy == 0 && dz == 0)	continue;
		offsets.emplace_back(dx, dy, dz);
	}
	return offsets;
}

Voxelizer::Label Voxelizer::GetLabel(const ivec3& index)
{
	if (m_labels.size() == 0) m_labels = CreateLabel();

	return (Voxelizer::Label)m_labels[index.x][index.y][index.z];
}

BDB Voxelizer::GetCellBDB(const ivec3& index) const
{
	auto pitch = GetPitch();
	Vector3 min = Vector3(pitch * index.x, pitch * index.y, pitch * index.z) + m_pNode->GetBoundBox().Min();
	
	Vector3 max = Vector3(pitch * index.x, pitch * index.y, pitch * index.z) + m_pNode->GetBoundBox().Min() + pitch;

	return BDB(min, max);
}
float Voxelizer::GetPitch() const
{
	return m_pNode->GetBoundBox().MaxLength() / m_resolution;
}

Vector3 Voxelizer::GetCenter(const ivec3& index) const
{
	auto pitch = GetPitch();
	return Vector3(
		pitch * index.x,
		pitch * index.y,
		pitch * index.z) + m_pNode->GetBoundBox().Min()
		+ (pitch / 2.0f);
}
bool Voxelizer::InVoxel(const ivec3& index) const
{
	return
		index.x >= 0 && index.x < m_resolution &&
		index.y >= 0 && index.y < m_resolution &&
		index.z >= 0 && index.z < m_resolution;
}

void Voxelizer::CreateLabelPoint(Vector<Vector3>& position, Vector<Vector3>& color, Voxelizer::Label type)
{
	auto labels = CreateLabel();
	for (int i = 0; i < m_resolution; i++)
	for (int j = 0; j < m_resolution; j++)
	for (int k = 0; k < m_resolution; k++) {
		auto label = labels[i][j][k];
		if (type == label) {
			position.push_back(GetCenter(ivec3(i, j, k)));
			color.push_back(ColorUtility::CreatePrimary((int)label));
		}
	}
}
Voxelizer::VoxelLabel Voxelizer::CreateLabel() const
{
	auto voxelArray = Create1DArray(m_resolution);
#ifdef DEBUG_VOXEL
	m_pCpuVoxel->GetBufferData(voxelArray.data(), voxelArray.size() * sizeof(unsigned int));
#else
	m_gpuVoxelizer.m_pVoxelBuffer->GetBufferData(voxelArray.data(), voxelArray.size() * sizeof(unsigned int));
#endif // DEBUG_VOXEL

	VoxelLabel label(m_resolution);
	for (int i = 0; i < m_resolution; i++) {
		label[i].resize(m_resolution);
		for (int j = 0; j < m_resolution; j++) {
			label[i][j].resize(m_resolution, Label::UNKNOWN);
			for (int k = 0; k < m_resolution; k++) {
				if (IsBoundary(voxelArray, m_resolution, ivec3(i, j, k))) {
					label[i][j][k] = Label::BOUNDARY;
				}
			}
		}
	}

	auto offsets = Create26Neighbor();

	for(int i = 0; i < m_resolution; i++)
	for(int j = 0; j < m_resolution; j++)
	for(int k = 0; k < m_resolution; k++) {
		if (label[i][j][k] != Label::UNKNOWN) { continue; }
		if (!(i == 0 || j == 0 || k == 0 || i == m_resolution - 1 || j == m_resolution - 1 || k == m_resolution - 1)) { continue; }
		std::queue<ivec3> q;
		q.push(ivec3(i, j, k));
		while (!q.empty()) {
			glm::ivec3 current = q.front();	q.pop();
			for (const auto& offset : offsets) {
				glm::ivec3 index = current + offset;
				if (InVoxel(index) &&
					label[index.x][index.y][index.z] == Label::UNKNOWN) {
					label[index.x][index.y][index.z] = Label::OUTER;
					q.push(index); 
				}
			}
		}
	}

	for (int i = 0; i < m_resolution; i++) 
	for (int j = 0; j < m_resolution; j++) 
	for (int k = 0; k < m_resolution; k++)
		if (label[i][j][k] == Label::UNKNOWN) {
			label[i][j][k] = Label::INNER;
		}

	return label;
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
	path.extension[SHADER_PROGRAM_COMPUTE].push_back("#extension GL_NV_gpu_shader5 : enable\n");
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
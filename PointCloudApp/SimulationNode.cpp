#include "SimulationNode.h"
#include "SimpleShader.h"
#include "Utility.h"
namespace KI
{

const float DT = 0.01f;
const Vector4 GRAVITY = Vector4(0.0f, -9.8f, 0.0f, 0.0f);
const float RADIUS = 2;
const float RADIUS2 = 2 * 2;
const float PRESSURE_STRENGTH = 200.0f;
const float VISCOSITY = 0.1f;
SimulationNode::SimulationNode()
	: RenderNode("Simulation")
    , m_debug(true)
    , m_simulationCount(0)
{
    m_useGpu[PHASE::POSITION] = true;
    m_useGpu[PHASE::DENSITY] = true;
    m_useGpu[PHASE::CELLID] = true;
    m_useGpu[PHASE::VELOCITY] = true;
    m_space = BDB(Vector3(-100, 0, -100), Vector3(100, 100, 100));
    auto size = m_space.Max() - m_space.Min();
    m_cellNum = Vector3i(size / RADIUS) + Vector3i(1);
    Initialize();
    m_grid.Allocate(m_cellNum.x * m_cellNum.y * m_cellNum.z, m_particles.size());
    SetBoundBox(m_space);
    if (m_useGpu.any()) {
        SettingGpu setting;
        setting.cellNum = Vector4i(m_cellNum.x, m_cellNum.y, m_cellNum.z, 0);
        setting.spaceMin = Vector4(m_space.Min(), 0);
        setting.spaceMax = Vector4(m_space.Max(), 0);
        setting.particleNum = m_particles.size();
        setting.gridNum = m_grid.Size();
        m_gpu.pSetting = std::make_unique<GLBuffer>();
        m_gpu.pSetting->Create(KI::DATA_UNKNOWN, 1, sizeof(setting), &setting);

        m_gpu.pGridCell = std::make_unique<GLBuffer>();
        m_gpu.pGridCell->Create<Vector4u>(m_grid.cell);

        m_gpu.pParticleIndex = std::make_unique<GLBuffer>();
        m_gpu.pParticleIndex->Create(m_grid.particleIndex);

        m_gpu.pParticle = std::make_unique<GLBuffer>();
        m_gpu.pParticle->Create<Particle>(m_particles);
    }

}

void SimulationNode::DrawNode(const DrawContext& context)
{
    Simulation();
    m_simulationCount++;
    if (m_useGpu[VELOCITY]) {
        m_gpu.pParticle->GetBufferData(m_particles.data(), sizeof(Particle) * m_particles.size());
    }

    Vector<Vector3> position(m_particles.size());
    Vector<Vector4> color(m_particles.size());


    for (int i = 0; i < m_particles.size(); i++) {
        position[i] = (Vector3(m_particles[i].position.x, m_particles[i].position.y, m_particles[i].position.z));
        color[i] = (Vector4(m_particles[i].velocity.x, m_particles[i].velocity.y, m_particles[i].velocity.z, 1.0f));
    }

    m_gpu.pPosition->BufferSubData(0, position);
    m_gpu.pVelocity->BufferSubData(0, color);


    const auto& pResourece = context.pResource;
    auto pPrimitiveColorShader = pResourece->GetShaderTable()->GetPrimitiveColorShader();
    pPrimitiveColorShader->Use();
    pPrimitiveColorShader->SetPosition(m_gpu.pPosition.get());
    pPrimitiveColorShader->SetColor(m_gpu.pVelocity.get());
    pPrimitiveColorShader->SetCamera(pResourece->GetCameraBuffer());
    pPrimitiveColorShader->SetModel(GetMatrix());
    pPrimitiveColorShader->DrawArray(GL_POINTS, m_gpu.pPosition.get());


}
void SimulationNode::ShowUI(UIContext& ui)
{
    if (ImGui::Button("Initialize")) {
        Initialize();
    }
}



void SimulationNode::Initialize()
{
    if (m_debug) {
        m_particles.resize(100);
        int sqrtCount = static_cast<int>(std::cbrt(m_particles.size()));
        Vector3 extent = m_space.Max() - m_space.Min();
        for (int i = 0; i < m_particles.size(); ++i) {
            int x = i % sqrtCount;
            int y = (i / sqrtCount) % sqrtCount;
            int z = i / (sqrtCount * sqrtCount);

            // ê≥ãKâªÇµÇƒãÛä‘ì‡Ç…é˚ÇﬂÇÈ
            Vector3 pos = m_space.Min() + extent * Vector3(
                static_cast<float>(x) / sqrtCount,
                static_cast<float>(y) / sqrtCount,
                static_cast<float>(z) / sqrtCount);
            m_particles[i].position = Vector4(pos.x, pos.y, pos.z, 0.0f);
            m_particles[i].velocity = Vector4(0.0f);
        }
    } else {
        m_particles.resize(10000);
        for (int i = 0; i < m_particles.size(); ++i) {
            m_particles[i].position = Vector4(Random::Vec3(m_space.Min(), m_space.Max()), 0.0f);
            m_particles[i].velocity = Vector4(0.0f);
        }
    }

    m_gpu.pPosition = std::make_unique<GLBuffer>();
    m_gpu.pVelocity = std::make_unique<GLBuffer>();
    m_gpu.pPosition->Create(m_particles.size(), sizeof(Vector3));
    m_gpu.pVelocity->Create(m_particles.size(), sizeof(Vector4));
}


Vector<int> SimulationNode::GetNeighbors(int target)
{
    Vector<int> neighbors;
    auto position = m_particles[target].GetPosition();
    auto cellIndex = GetCellID(position);
	for (int i = -1; i <= 1; i++)
    for (int j = -1; j <= 1; j++)
	for (int k = -1; k <= 1; k++) {
        auto dIndex = Vector3i(cellIndex.x + i, cellIndex.y + j, cellIndex.z + k);
        if (dIndex.x < 0 || dIndex.y < 0 || dIndex.z < 0) { continue; }
        if (dIndex.x >= m_cellNum.x || dIndex.y >= m_cellNum.y || dIndex.z >= m_cellNum.z) { continue; }
        auto index1D = ToIndex1D(dIndex);
        for (int offset = 0; offset < m_grid.Num(index1D); offset++) {
            int other = m_grid.ParticleIndex(m_grid.Offset(index1D) + offset);
            if (target == other) continue;
            const auto& pos2 = m_particles[other].GetPosition();
            float dist = glm::distance(position, pos2);
            if (dist < RADIUS) {
                neighbors.push_back(other);
            }
        }
    }
    return neighbors;
}

void SimulationNode::Simulation()
{
    if (m_useGpu.any()) {
        IShader::BindShaderStorage(0, m_gpu.pSetting->Handle());
        IShader::BindShaderStorage(1, m_gpu.pParticle->Handle());
        IShader::BindShaderStorage(2, m_gpu.pGridCell->Handle());
        IShader::BindShaderStorage(3, m_gpu.pParticleIndex->Handle());
    }

    UpdatePosition();
    UpdateDensity();
    UpdateCellID();
    UpdateVelocity();
}

void SimulationNode::TestPosition()
{
    for (int i = 0; i < m_particles.size(); i++) {
        const auto& pos = m_particles[i].position;
        if (pos.x < m_space.Min().x ||
            pos.y < m_space.Min().y ||
            pos.z < m_space.Min().z) {
            assert(0);
        }

        if (pos.x > m_space.Max().x ||
            pos.y > m_space.Max().y ||
            pos.z > m_space.Max().z) {
            assert(0);
        }

    }
}
void SimulationNode::UpdatePosition()
{
    if (m_useGpu[POSITION]) {
        if (m_gpu.pPositionUpdater == nullptr) {
            m_gpu.pPositionUpdater = std::make_unique<PositionUpdater>();
            m_gpu.pPositionUpdater->Build();
        }
        m_gpu.pPositionUpdater->Use();
        m_gpu.pPositionUpdater->Dispatch1D(m_particles.size());
        IComputeShader::BarrierSSBO();


        if (!m_useGpu[DENSITY] ||
            !m_useGpu[CELLID]) {
            m_gpu.pParticle->GetBufferData(m_particles.data(), sizeof(Particle) * m_particles.size());
            TestPosition();
        }

        return;
    }


    for (auto& p : m_particles) {
        p.velocity += GRAVITY * DT;
        p.position += p.velocity * DT;

        // ï«è’ìÀ
        for (int d = 0; d < 3; ++d) {
            if (p.position[d] < m_space.Min()[d]) {
                p.position[d] = m_space.Min()[d];
                p.velocity[d] *= -0.5f;
            }
            if (p.position[d] > m_space.Max()[d]) {
                p.position[d] = m_space.Max()[d];
                p.velocity[d] *= -0.5f;
            }
        }
    }

    if (m_useGpu[VELOCITY]) {
        m_gpu.pParticle->BufferSubData(0, m_particles);
    }
}


void SimulationNode::UpdateDensity()
{
    if (m_useGpu[DENSITY]) {
        if (m_gpu.pDensityUpdater == nullptr) {
            m_gpu.pDensityUpdater = std::make_unique<DensityUpdater>();
            m_gpu.pDensityUpdater->Build();
        }
        m_gpu.pDensityUpdater->Use();
        m_gpu.pDensityUpdater->Dispatch1D(m_particles.size());
        IComputeShader::BarrierSSBO();

        if (!m_useGpu[CELLID]) {
            m_gpu.pGridCell->GetBufferData(m_grid.cell.data(), m_grid.cell.size() * sizeof(Vector4u));
        }
        return;
    }



    auto radius = Vector3(RADIUS, RADIUS, RADIUS);
    for (int i = 0; i < m_particles.size(); i++) {
        auto p = m_particles[i].GetPosition();

        auto minCell = GetCellID(p - radius);
        auto maxCell = GetCellID(p + radius);

        for (int x = minCell.x; x <= maxCell.x; x++)
            for (int y = minCell.y; y <= maxCell.y; y++)
                for (int z = minCell.z; z <= maxCell.z; z++) {
                    if (x < 0 || y < 0 || z < 0) { continue; }
                    if (x >= m_cellNum.x || y >= m_cellNum.y || z >= m_cellNum.z) { continue; }

                    auto center = GetCellCenter(Vector3i(x, y, z));
                    auto dist2 = glm::length2(center - p);
                    if (dist2 < RADIUS2) {
                        m_grid.AddDensity(ToIndex1D(Vector3i(x, y, z)), Poly6FromR2(dist2, RADIUS2));
                    }
                }
    }


    if (m_useGpu[CELLID]) {
        m_gpu.pGridCell->BufferSubData(0, m_grid.cell);
    }
}

void SimulationNode::UpdateCellID()
{
    if (m_useGpu[CELLID]) {
        m_gpu.pGridCell->SetData(0);
        m_gpu.pParticleIndex->SetData(0);
        if (m_gpu.pCellIDPrefixSum[0] == nullptr) {
            m_gpu.pCellIDPrefixSum[0] = std::make_unique<CellIDUpdaterPrefixSum>(1); m_gpu.pCellIDPrefixSum[0]->Build();
            m_gpu.pCellIDPrefixSum[1] = std::make_unique<CellIDUpdaterPrefixSum>(2); m_gpu.pCellIDPrefixSum[1]->Build();
            m_gpu.pCellIDPrefixSum[2] = std::make_unique<CellIDUpdaterPrefixSum>(3); m_gpu.pCellIDPrefixSum[2]->Build();
            m_gpu.pCellIDUpdater[0] = std::make_unique<CellIDUpdater>(1); m_gpu.pCellIDUpdater[0]->Build();
            m_gpu.pCellIDUpdater[1] = std::make_unique<CellIDUpdater>(2); m_gpu.pCellIDUpdater[1]->Build();
            m_gpu.pPrefixSumChunk = std::make_unique<GLBuffer>();
            m_gpu.pPrefixSumChunk->Create(CellIDUpdaterPrefixSum::GetBlockNum(), sizeof(unsigned int));
            m_gpu.pGridOffset = std::make_unique<GLBuffer>();
            m_gpu.pGridOffset->Create(m_grid.Size(), sizeof(unsigned int));

        }

        m_gpu.pCellIDUpdater[0]->Use();
        m_gpu.pCellIDUpdater[0]->Dispatch1D(m_particles.size());
        IComputeShader::BarrierSSBO();

        m_gpu.pCellIDPrefixSum[0]->Use();
        m_gpu.pCellIDPrefixSum[0]->Dispatch1D((m_particles.size() / m_gpu.pCellIDPrefixSum[0]->GetLocalThreadNum().x) + 1);
        IComputeShader::BarrierSSBO();

        m_gpu.pPrefixSumChunk->SetData(0);
        IShader::BindShaderStorage(4, m_gpu.pPrefixSumChunk->Handle());
        m_gpu.pCellIDPrefixSum[1]->Use();
        m_gpu.pCellIDPrefixSum[1]->Dispatch1D(1);
        IComputeShader::BarrierSSBO();

        m_gpu.pCellIDPrefixSum[2]->Use();
        m_gpu.pCellIDPrefixSum[2]->Dispatch1D(m_particles.size());
        IComputeShader::BarrierSSBO();

        m_gpu.pGridOffset->SetData(0);
        IShader::BindShaderStorage(4, m_gpu.pGridOffset->Handle());
        m_gpu.pCellIDUpdater[1]->Use();
        m_gpu.pCellIDUpdater[1]->Dispatch1D(m_particles.size());
        IComputeShader::BarrierSSBO();

        return;
    }

    m_grid.Initialize();

    for (int i = 0; i < m_particles.size(); i++) {
        const auto& p = m_particles[i];
        auto cellID = GetCellID(p.GetPosition());
        m_grid.AddNum(ToIndex1D(cellID));
    }

    for (size_t i = 1; i < m_grid.Size(); i++) {
        m_grid.SetOffset(i, m_grid.Offset(i - 1) + m_grid.Num(i - 1));
    }

    Vector<int> offset(m_grid.Size(), 0);
    for (size_t i = 0; i < m_particles.size(); i++) {
        const auto& p = m_particles[i];
        auto cellID = GetCellID(p.GetPosition());
        auto cellIndex = ToIndex1D(cellID);
        m_grid.SetParticleIndex(m_grid.Offset(cellIndex) + offset[cellIndex], i);
        offset[cellIndex]++;
    }

    if (m_useGpu[VELOCITY]) {
        m_gpu.pGridCell->BufferSubData(0, m_grid.cell);
        m_gpu.pParticleIndex->BufferSubData(0, m_grid.particleIndex);
    }
}

void SimulationNode::UpdateVelocity()
{
    if (m_useGpu[VELOCITY]) {
        if (m_gpu.pVelocityUpdater == nullptr) {
            m_gpu.pVelocityUpdater = std::make_unique<VelocityUpdater>();
            m_gpu.pVelocityUpdater->Build();
        }
        m_gpu.pVelocityUpdater->Use();
        m_gpu.pVelocityUpdater->Dispatch1D(m_particles.size());
        IComputeShader::BarrierSSBO();
        return;
    }

    // ä»à’à≥óÕÇ∆îΩî≠
    for (int i = 0; i < m_particles.size(); ++i) {
        auto& pi = m_particles[i];
        auto neighbors = GetNeighbors(i);
        Vector4 pressure_force(0.0f);
        for (int j : neighbors) {
            auto& pj = m_particles[j];
            auto dir = pi.position - pj.position;
            float dist = glm::length(dir);
            if (dist > 0.0f) {
                dir /= dist;
                float overlap = RADIUS - dist;
                pressure_force += dir * overlap * PRESSURE_STRENGTH;
            }
        }
        m_particles[i].velocity += pressure_force * DT;
    }
}

Vector3i SimulationNode::GetCellID(const Vector3& pos)
{
    Vector3 position = pos - m_space.Min();
    return Vector3i(glm::floor(position / RADIUS));
}

int SimulationNode::ToIndex1D(const Vector3i& cellID)
{
    return
        cellID.z * m_cellNum.y * m_cellNum.x +
        cellID.y * m_cellNum.x +
        cellID.x;
}


Vector3 SimulationNode::GetCellCenter(const Vector3i& id)
{
    return Vector3(m_space.Min() + (Vector3(id.x, id.y, id.z) + Vector3(0.5f)) * RADIUS);
}

float SimulationNode::Poly6FromR2(float r, float h)
{
    if (r >= 0 && r <= h) {
        float x = (h * h - r * r);
        return 315.0f / (64.0f * glm::pi<float>() * pow(h, 9)) * x * x * x;
    } else {
        return 0.0f;
    }
}

ShaderPath SimulationNode::PositionUpdater::GetShaderPath()
{
    ShaderPath path;
    path.version = "version.h";
    path.header.push_back("simulation\\common.h");
    path.shader[SHADER_PROGRAM_COMPUTE] = "simulation\\updatePosition.comp";
    return path;
}

ShaderPath SimulationNode::VelocityUpdater::GetShaderPath()
{
    ShaderPath path;
    path.version = "version.h";
    path.header.push_back("simulation\\common.h");
    path.shader[SHADER_PROGRAM_COMPUTE] = "simulation\\updateVelocity.comp";
    return path;
}

ShaderPath SimulationNode::DensityUpdater::GetShaderPath()
{
    ShaderPath path;
    path.version = "version.h";
    path.header.push_back("simulation\\common.h");
    path.extension[SHADER_PROGRAM_COMPUTE].push_back("#extension GL_NV_shader_atomic_float : require\n");
    path.shader[SHADER_PROGRAM_COMPUTE] = "simulation\\updateDensity.comp";
    return path;
}

ShaderPath SimulationNode::CellIDUpdater::GetShaderPath()
{
    ShaderPath path;
    path.version = "version.h";
    path.header.push_back("simulation\\common.h");
    if (m_phase == 1) {
        path.extension[SHADER_PROGRAM_COMPUTE].push_back("#define PHASE_1\n");
    } else if(m_phase == 2) {
        path.extension[SHADER_PROGRAM_COMPUTE].push_back("#define PHASE_2\n");
    } else {
        assert(0);
    }
    path.shader[SHADER_PROGRAM_COMPUTE] = "simulation\\updateCellID.comp";
    return path;
}

Vector3i SimulationNode::CellIDUpdaterPrefixSum::GetLocalThreadNum() const
{
    if (m_phase == 1 || m_phase == 3) {
        return Vector3i(GetBlockNum(), 1, 1);
    }

    return Vector3i(1, 1, 1);
}
ShaderPath SimulationNode::CellIDUpdaterPrefixSum::GetShaderPath()
{
    ShaderPath path;
    path.version = "version.h";
    path.header.push_back("simulation\\common.h");
    if (m_phase == 1) {
        path.extension[SHADER_PROGRAM_COMPUTE].push_back("#define PHASE_1\n");
    } else if (m_phase == 2) {
        path.extension[SHADER_PROGRAM_COMPUTE].push_back("#define PHASE_2\n");
    } else if (m_phase == 3) {
        path.extension[SHADER_PROGRAM_COMPUTE].push_back("#define PHASE_3\n");
    } else {
        assert(0);
    }

    path.shader[SHADER_PROGRAM_COMPUTE] = "simulation\\updateCellID_prefixSum.comp";
    return path;
}

}
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
{
    m_space = BDB(Vector3(-100, 0, -100), Vector3(100, 100, 100));
    auto size = m_space.Max() - m_space.Min();
    m_cellNum = Vector3i(size / RADIUS) + Vector3i(1);
    SetBoundBox(m_space);
    Initialize();

}

void SimulationNode::DrawNode(const DrawContext& context)
{
    Simulation();
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
    m_particles.resize(10000);
    for (int i = 0; i < m_particles.size(); ++i) {
        m_particles[i].position = Vector4(Random::Vec3(m_space.Min(), m_space.Max()), 0.0f);
        m_particles[i].velocity = Vector4(0.0f);
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
    UpdatePosition();
    UpdateDensity();
    UpdateCellID();
    UpdateVelocity();
}

void SimulationNode::UpdatePosition()
{
    for (auto& p : m_particles) {
        p.velocity += GRAVITY * DT;
        p.position += p.velocity * DT;

        // •ÇÕ“Ë
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
}

void SimulationNode::UpdateVelocity()
{
    // ŠÈˆÕˆ³—Í‚Æ”½”­
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
void SimulationNode::UpdateCellID()
{
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
void SimulationNode::UpdateDensity()
{
    m_grid.Allocate(m_cellNum.x * m_cellNum.y * m_cellNum.z, m_particles.size());
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
}

}
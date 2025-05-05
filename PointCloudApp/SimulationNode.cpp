#include "SimulationNode.h"
#include "SimpleShader.h"
#include "Utility.h"
namespace KI
{
SimulationNode::SimulationNode()
	: RenderNode("Simulation")
{
    m_gpu.pPosition = std::make_unique<GLBuffer>();
    m_gpu.pVelocity = std::make_unique<GLBuffer>();
    Initialize();
}

void SimulationNode::DrawNode(const DrawContext& context)
{
    Simulation();
    Vector<Vector3> position(m_particles.size());
    Vector<Vector4> color(m_particles.size());

    for (int i = 0; i < m_particles.size(); i++) {
        position.push_back(Vector3(m_particles[i].position.x, m_particles[i].position.y, m_particles[i].position.z));
        color.push_back(Vector4(m_particles[i].velocity.x, m_particles[i].velocity.y, m_particles[i].velocity.z, 1.0f));
    }
    m_gpu.pPosition->Create(position);
    m_gpu.pVelocity->Create(color);


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
    m_particles.resize(5000);
    for (int i = 0; i < m_particles.size(); ++i) {
        m_particles[i].position = Vector4(Random::Vec3(0, 100), 0.0f);
        m_particles[i].velocity = Vector4(0.0f);
    }

    /*int size = std::cbrt(m_particles.size());
    int index = 0;
    for (int x = 0; x < size; ++x) 
    for (int y = 0; y < size; ++y)
    for (int z = 0; z < size; ++z) {
        m_particles[index].position = Vector4(x * 2, y * 2 + 50, z * 2,0.0f);
        m_particles[index].velocity = Vector4(0.0f);
	    ++index;
	}*/
}

const float DT = 0.01f;
const Vector4 GRAVITY = Vector4(0.0f, -9.8f, 0.0f, 0.0f);
const float BOUND = 100.0f;
const float RADIUS = 2;
const float REST_DENSITY = 1000.0f;
const float PRESSURE_STRENGTH = 200.0f;
const float VISCOSITY = 0.1f;

Vector<int> SimulationNode::GetNeighbors(int i)
{
    Vector<int> neighbors;
    for (int j = 0; j < m_particles.size(); ++j) {
        if (i == j) continue;
        float dist = glm::distance(m_particles[i].position, m_particles[j].position);
        if (dist < RADIUS) {
            neighbors.push_back(j);
        }
    }
    return neighbors;
}

void SimulationNode::Simulation()
{
    for (auto& p : m_particles) {
        p.velocity += GRAVITY * DT;
        p.position += p.velocity * DT;

        // •ÇÕ“Ë
        for (int d = 0; d < 3; ++d) {
            if (p.position[d] < 0.0f) {
                p.position[d] = 0.0f;
                p.velocity[d] *= -0.5f;
            }
            if (p.position[d] > BOUND) {
                p.position[d] = BOUND;
                p.velocity[d] *= -0.5f;
            }
        }
    }

    // ŠÈˆÕˆ³—Í‚Æ”½”­
    std::vector<Vector4> corrections(m_particles.size(), Vector4(0.0f));

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
        corrections[i] = pressure_force;
    }

    for (int i = 0; i < m_particles.size(); ++i) {
        m_particles[i].velocity += corrections[i] * DT;
    }
}

}
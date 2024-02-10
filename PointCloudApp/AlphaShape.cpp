#include "AlphaShape.h"
#include "PointCloudNode.h"
#include "PrimitiveNode.h"
#include "PointCloud.h"
#include "Utility.h"
#include "glm/gtx/norm.hpp"
AlphaShape2D::AlphaShape2D(PointCloudNode* pPointCloud)
	:m_pPointCloud(pPointCloud)
    ,m_alpha(1.0f)
{
}

AlphaShape2D::~AlphaShape2D()
{
}

void AlphaShape2D::Execute()
{
    m_edges.clear();
    float alpha = m_ui.alpha;
    float alpha_2 = alpha * alpha;
    const auto& points = m_pPointCloud->GetData()->Position();
    // 1. run through all pairs of points
    for (int i = 0; i < points.size() - 1; i++) {
        for (int j = i + 1; j < points.size(); j++) {
            if (points[i] == points[j]) { assert(0); } // alternatively, continue
            auto dist = glm::distance(points[i], points[j]);
            if (dist > 2 * alpha) { continue; } // circle fits between points ==> p_i, p_j can't be alpha-exposed                    

            // find two circles that contain p_i and p_j; note that center1 == center2 if dist == 2*alpha
            auto alphaDist = (dist / 2);
            alphaDist *= alphaDist;
            alphaDist = std::sqrtf(alpha_2 - alphaDist);

            vec3 diff = vec3(alphaDist * (points[i].y - points[j].y) / dist, alphaDist * (points[j].x - points[i].x) / dist, 0.0f);
            auto mid = (points[i] + points[j]) * 0.5f;
            auto center1 = mid + diff;
            auto center2 = mid - diff;

            // check if one of the circles is alpha-exposed, i.e. no other point lies in it
            bool c1_empty = true, c2_empty = true;
            for (int k = 0; k < points.size() && (c1_empty || c2_empty); k++) {
                if (points[k] == points[i] || points[k] == points[j]) { continue; }
                if (glm::distance2(center1, points[k]) < alpha_2) { c1_empty = false; }
                if (glm::distance2(center2, points[k]) < alpha_2) { c2_empty = false; }
            }

            if (c1_empty || c2_empty) {
                // yup!
                m_edges.push_back(Edge(points[i], points[j]));
            }
        }
    }
}

void AlphaShape2D::ShowUI()
{
    ImGui::Text("AlphaShape");
    if (ImGui::SliderFloat("Alpha", &m_ui.alpha, 0.0f, 80.0f, "%lf")) {
        Execute(); 
        auto pPoint = std::make_shared<Primitive>();
        Vector<vec3> pos(m_edges.size() * 2);
        int i = 0;
        for (const auto& edge : m_edges) {
            pos[i++] = edge.begin;
            pos[i++] = edge.end;
        }
        pPoint->SetPosition(std::move(pos));
        pPoint->SetType(GL_LINES);
        m_pPointCloud->SetNode(std::make_shared<PrimitiveNode>("AlphaLine", pPoint, ColorUtility::CreatePrimary(4)));
    }
}
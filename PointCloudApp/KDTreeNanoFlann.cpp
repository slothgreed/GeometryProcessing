#include "KDTreeNanoFlann.h"
#include "PointCloudNode.h"
#include "PointCloud.h"
#include "Utility.h"
#include "PrimitiveNode.h"
#include "Primitives.h"
#include "../extlibs/nanoflann/nanoflann.hpp"

struct Vector3Adaptor
{
	std::vector<vec3> rawPoints;
	inline uint32_t kdtree_get_point_count() const { return rawPoints.size(); }
	inline float kdtree_get_pt(uint32_t idx, int32_t dim) const { return rawPoints[idx][dim]; }
	template<class BBOX>
	bool kdtree_get_bbox(BBOX& bb) const { return false; }
};

class NanoFlannImpl : public KDTreeNanoFlann::Impl
{
public:
	NanoFlannImpl(PointCloudNode* pNode)
		: data{ pNode->GetData()->Position() }
		, tree(3, data)
	{
		tree.buildIndex();
	}
	~NanoFlannImpl() {};

	typedef nanoflann::KDTreeSingleIndexAdaptor<
		nanoflann::L2_Simple_Adaptor<float, Vector3Adaptor>, Vector3Adaptor, 3>
		KDTree_T;


	std::vector<uint32_t> Nearest(const vec3 query, size_t k)
	{
		std::vector<uint32_t> outInds(k);
		std::vector<float> outDistSq(k);
		tree.knnSearch(&query[0], k, &outInds[0], &outDistSq[0]);
		return outInds;
	}

	std::vector<uint32_t> RadiusSearch(const vec3 query, double rad)
	{
		double radSq = rad * rad;
		std::vector<nanoflann::ResultItem<uint32_t, float>> outPairs;
		tree.radiusSearch(&query[0], radSq, outPairs, nanoflann::SearchParameters());

		std::vector<uint32_t> outInds(outPairs.size());
		for (size_t i = 0; i < outInds.size(); i++) {
			outInds[i] = outPairs[i].first;
		}

		return outInds;
	}
private:
	Vector3Adaptor data;
	KDTree_T tree;
};

KDTreeNanoFlann::KDTreeNanoFlann(PointCloudNode* pointCloud, int dimension)
	: m_pImpl(nullptr)
	, m_pPointCloud(pointCloud)
{
}

KDTreeNanoFlann::~KDTreeNanoFlann()
{
}

void KDTreeNanoFlann::Execute()
{
	if (m_pImpl) {
		delete m_pImpl;
	}
	m_pImpl = new NanoFlannImpl(m_pPointCloud);
}


void KDTreeNanoFlann::ShowUI()
{
	if (ImGui::Button("KDTree")) {
		if (!m_pImpl) {
			Execute();
		}
	}

	if (!m_pImpl) {
		return;
	}

	if (ImGui::Button("Find")) {
		auto pNanoFlann = (NanoFlannImpl*)m_pImpl;
		for (int i = 0; i < 20; i++) {
			vec3 target = Random::Vec3(0, 100);
			auto result = pNanoFlann->Nearest(target, 1);

			const auto& ret = m_pPointCloud->GetData()->Position()[result[0]];
			auto pPoint = std::make_shared<Primitive>();
			std::vector<vec3> pos;
			pos.push_back(target);
			pos.push_back(ret);
			pPoint->SetPosition(std::move(pos));
			pPoint->SetType(GL_LINES);
			m_pPointCloud->SetNode(std::make_shared<PrimitiveNode>("Line" + std::to_string(i), pPoint, ColorUtility::CreatePrimary(i)));

			std::shared_ptr<Primitive> pCircle = std::make_shared<Circle>(glm::length(target - ret), target);
			m_pPointCloud->SetNode(std::make_shared<PrimitiveNode>("Range" + std::to_string(i), pCircle, ColorUtility::CreatePrimary(i)));
			Printf::Vec3(std::to_string(i), target);
		}
	}

	if (ImGui::Button("FindSphere")) {
		auto pNanoFlann = (NanoFlannImpl*)m_pImpl;
		auto indexs = pNanoFlann->RadiusSearch(m_pPointCloud->GetData()->Position()[Random::Int(0,m_pPointCloud->GetData()->Position().size())], m_ui.slider);
		
		std::vector<vec3> color(m_pPointCloud->GetData()->Position().size(), vec3(1, 1, 1));
		for (auto index : indexs) {
			color[index] = ColorUtility::CreatePrimary(4);
		}
		m_pPointCloud->GetData()->SetColor(std::move(color));
		m_pPointCloud->GetData()->Update();
	}

	ImGui::SliderInt("SphereRange", &m_ui.slider, 0, 100, "%d");
}
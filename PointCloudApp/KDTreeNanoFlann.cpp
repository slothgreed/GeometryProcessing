#include "KDTreeNanoFlann.h"
#include "PointCloudNode.h"
#include "PointCloud.h"
#include "Utility.h"
#include "PrimitiveNode.h"
#include "Primitives.h"
#include "../extlibs/nanoflann/nanoflann.hpp"
namespace KI
{
struct Vector3Adaptor
{
	Vector<Vector3> rawPoints;
	inline int kdtree_get_point_count() const { return (int)rawPoints.size(); }
	inline float kdtree_get_pt(int idx, int dim) const { return rawPoints[idx][dim]; }
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
		nanoflann::L2_Simple_Adaptor<float, Vector3Adaptor, float, int>, Vector3Adaptor, 3, int>
		KDTree_T;


	Vector<int> Nearest(const Vector3& query, size_t k)
	{
		Vector<int> outInds(k);
		Vector<float> outDistSq(k);
		tree.knnSearch(&query[0], k, &outInds[0], &outDistSq[0]);
		return outInds;
	}

	Vector<int> RadiusSearch(const Vector3& query, float rad)
	{
		double radSq = rad * rad;
		Vector<nanoflann::ResultItem<int, float>> outPairs;
		tree.radiusSearch(&query[0], radSq, outPairs, nanoflann::SearchParameters());

		Vector<int> outInds(outPairs.size());
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
	if (!m_pImpl) {
		m_pImpl = new NanoFlannImpl(m_pPointCloud);
	}
}
Vector<int> KDTreeNanoFlann::GetRadiusNeighbor(const Vector3& query, float rad)
{
	auto pNanoFlann = (NanoFlannImpl*)m_pImpl;
	return pNanoFlann->RadiusSearch(query, rad);
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
			Vector3 target = Random::Vec3(0, 100);
			auto result = pNanoFlann->Nearest(target, 1);

			const auto& ret = m_pPointCloud->GetData()->Position()[result[0]];
			auto pPoint = std::make_shared<Primitive>();
			Vector<Vector3> pos;
			pos.push_back(target);
			pos.push_back(ret);
			pPoint->SetPosition(std::move(pos));
			pPoint->SetType(GL_LINES);
			m_pPointCloud->AddNode(std::make_shared<PrimitiveNode>("Line" + std::to_string(i), pPoint, ColorUtility::CreatePrimary(i)));

			Shared<Primitive> pCircle = std::make_shared<Circle>(glm::length(target - ret), target);
			m_pPointCloud->AddNode(std::make_shared<PrimitiveNode>("Range" + std::to_string(i), pCircle, ColorUtility::CreatePrimary(i)));
			Printf::Vec3(std::to_string(i), target);
		}
	}

	if (ImGui::Button("FindSphere")) {
		auto pNanoFlann = (NanoFlannImpl*)m_pImpl;
		auto indexs = pNanoFlann->RadiusSearch(m_pPointCloud->GetData()->Position()[Random::Int(0,m_pPointCloud->GetData()->Position().size())], m_ui.slider);
		
		Vector<Vector3> color(m_pPointCloud->GetData()->Position().size(), Vector3(1, 1, 1));
		for (auto index : indexs) {
			color[index] = ColorUtility::CreatePrimary(4);
		}
		m_pPointCloud->GetData()->SetColor(std::move(color));
		m_pPointCloud->GetData()->Update();
	}

	ImGui::SliderInt("SphereRange", &m_ui.slider, 0, 100, "%d");
}
}
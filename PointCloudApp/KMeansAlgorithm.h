#ifndef KMEAN_ALGORITHM_H
#define KMEAN_ALGORITHM_H
#include "IAlgorithm.h"

class PointCloud;
class KMeansAlgorithm : public IAlgorithm
{
public:

	KMeansAlgorithm(const std::shared_ptr<PointCloud>& pointCloud, int clusterNum, int iterateNum)
		:m_pointCloud(pointCloud)
		,m_clusterNum(clusterNum)
		,m_iterateNum(iterateNum)
	{};
	~KMeansAlgorithm() {};

	virtual void Execute();

	std::vector<vec3> CreateClusterColor();
	std::vector<vec3> CreateSeedColor();
	std::vector<std::vector<int>>&& GetResult() { return std::move(m_result); }
private:
	typedef std::vector<vec3> Seeds;
	typedef std::vector<std::vector<int>> Clusters;
	Seeds CreateInitSeed();
	void Calculate(const Seeds& seed, Clusters& result, Seeds& newSeed);
	std::shared_ptr<PointCloud> m_pointCloud;
	int m_clusterNum;
	int m_iterateNum;
	Clusters m_result;
};

#endif KMEAN_ALGORITHM_H
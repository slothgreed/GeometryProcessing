#ifndef KMEAN_ALGORITHM_H
#define KMEAN_ALGORITHM_H
#include "IAlgorithm.h"

class PointCloud;
class KMeansAlgorithm : public IAlgorithm
{
public:

	KMeansAlgorithm(const Shared<PointCloud>& pointCloud, int clusterNum, int iterateNum)
		:m_pointCloud(pointCloud)
		,m_clusterNum(clusterNum)
		,m_iterateNum(iterateNum)
	{};
	~KMeansAlgorithm() {};

	virtual void Execute();

	Vector<vec3> CreateClusterColor();
	Vector<vec3> CreateSeedColor();
	Vector<Vector<int>>&& GetResult() { return std::move(m_result); }
private:
	typedef Vector<vec3> Seeds;
	typedef Vector<Vector<int>> Clusters;
	Seeds CreateInitSeed();
	void Calculate(const Seeds& seed, Clusters& result, Seeds& newSeed);
	Shared<PointCloud> m_pointCloud;
	int m_clusterNum;
	int m_iterateNum;
	Clusters m_result;
};

#endif KMEAN_ALGORITHM_H
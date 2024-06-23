#include "KMeansAlgorithm.h"
#include "Random.h"
#include "Utility.h"
#include "PointCloud.h"
#include <numeric>
namespace KI
{
void KMeansAlgorithm::Execute(const Vector<Vector3>& position, int clusterNum, int iterateNum)
{
	m_positionNum = position.size();
	auto seeds = CreateInitSeed(position);

	for (int i = 0; i < iterateNum - 1; i++)
	{
		Clusters clusters;
		Seeds newSeeds;
		Calculate(position, seeds, clusters, newSeeds);
		seeds = std::move(newSeeds);
	}

	Seeds newSeeds;
	Calculate(position, seeds, m_result, newSeeds);
}

KMeansAlgorithm::Seeds KMeansAlgorithm::CreateInitSeed(const Vector<Vector3>& position)
{
	Seeds seeds;
	seeds.resize(m_clusterNum);
	int clusterSize = position.size() / (m_clusterNum + 1);
	int current = 0;
	for (int i = 0; i < m_clusterNum; i++) {
		seeds[i] = position[i * clusterSize];
	}

	return seeds;
}

void KMeansAlgorithm::Calculate(const Vector<Vector3>& positions, const Seeds& seed, Clusters& result, Seeds& newSeed)
{
	result.resize(m_clusterNum);
	newSeed.resize(m_clusterNum);
	for (int i = 0; i < positions.size(); i++) {
		const auto& position = positions[i];
		float minDist = std::numeric_limits<float>::infinity();
		int minSeed = 0;
		for (int j = 0; j < seed.size(); j++) {
			float dist = (float)glm::distance(position, seed[j]);
			if (dist < minDist) {
				minDist = dist;
				minSeed = j;
			}
		}

		newSeed[minSeed] += position;
		result[minSeed].push_back(i);
	}

	for (int j = 0; j < newSeed.size(); j++) {
		newSeed[j] /= result[j].size();
	}

}
Vector<Vector3> KMeansAlgorithm::CreateClusterColor()
{
	auto seedColor = CreateSeedColor();
	Vector<Vector3> color(m_positionNum);
	for (int i = 0; i < m_result.size(); i++) {
		for (int j = 0; j < m_result[i].size(); j++) {
			color[m_result[i][j]] = seedColor[i];
		}
	}

	return color;
}
Vector<Vector3> KMeansAlgorithm::CreateSeedColor()
{
	Vector<Vector3> seedColor(m_clusterNum);
	for (int i = 0; i < seedColor.size(); i++)
	{
		seedColor[i] = ColorUtility::CreateRandom();
	}

	return seedColor;
}
}
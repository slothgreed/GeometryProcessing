#include "KMeansAlgorithm.h"
#include "Random.h"
#include "Utility.h"
#include "PointCloud.h"
#include <numeric>
void KMeansAlgorithm::Execute()
{
	Clusters group;
	auto seeds = CreateInitSeed();

	for (int i = 0; i < m_iterateNum - 1; i++) 
	{
		Clusters clusters;
		Seeds newSeeds;
		Calculate(seeds, clusters, newSeeds);
		seeds = std::move(newSeeds);
	}

	Seeds newSeeds;
	Calculate(seeds, m_result, newSeeds);
}

KMeansAlgorithm::Seeds KMeansAlgorithm::CreateInitSeed()
{
	Seeds seeds;
	seeds.resize(m_clusterNum);
	int clusterSize = m_pointCloud->Position().size() / (m_clusterNum + 1);
	int current = 0;
	for (int i = 0; i < m_clusterNum; i++)
	{
		seeds[i] = m_pointCloud->Position().at(i * clusterSize);
	}

	return seeds;
}
void KMeansAlgorithm::Calculate(const Seeds& seed, Clusters& result, Seeds& newSeed)
{
	result.resize(m_clusterNum);
	newSeed.resize(m_clusterNum);
	for (int i = 0; i < m_pointCloud->Position().size(); i++)
	{
		const auto& position = m_pointCloud->Position().at(i);
		float minDist = std::numeric_limits<float>::infinity();
		int minSeed = 0;
		for (int j = 0; j < seed.size(); j++)
		{
			float dist = (float)glm::distance(position, seed[j]);
			if (dist < minDist)
			{
				minDist = dist;
				minSeed = j;
			}
		}

		newSeed[minSeed] += position;
		result[minSeed].push_back(i);
	}

	for (int j = 0; j < newSeed.size(); j++)
	{
		newSeed[j] /= result[j].size();
	}

}
std::vector<vec3> KMeansAlgorithm::CreateClusterColor()
{
	auto seedColor = CreateSeedColor();
	std::vector<vec3> color(m_pointCloud->Position().size());
	for (int i = 0; i < m_result.size(); i++)
	{
		for (int j = 0; j < m_result[i].size(); j++)
		{
			color[m_result[i][j]] = seedColor[i];
		}
	}

	return color;
}
std::vector<vec3> KMeansAlgorithm::CreateSeedColor()
{
	std::vector<vec3> seedColor(m_clusterNum);
	for (int i = 0; i < seedColor.size(); i++)
	{
		seedColor[i] = ColorUtility::CreateRandom();
	}

	return seedColor;
}

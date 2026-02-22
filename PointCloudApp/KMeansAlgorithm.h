#ifndef KMEAN_ALGORITHM_H
#define KMEAN_ALGORITHM_H
namespace KI
{
class KMeansAlgorithm
{
public:

	KMeansAlgorithm()
		: m_clusterNum(0)
		, m_positionNum(0)
	{
	};
	~KMeansAlgorithm() {};

	void Execute(const Vector<Vector3>& position, int clusterNum, int iterateNum);

	Vector<Vector3> CreateClusterColor();
	Vector<Vector<int>>&& GetResult() { return std::move(m_result); }
	const Vector<Vector3>& GetSeeds() const { return m_seeds; }
private:
	Vector<Vector3> CreateSeedColor();
	typedef Vector<Vector3> Seeds;
	typedef Vector<Vector<int>> Clusters;
	Seeds CreateInitSeed(const Vector<Vector3>& position);
	void Calculate(const Vector<Vector3>& positions, const Seeds& seed, Clusters& result, Seeds& newSeed);
	int m_clusterNum;
	int m_positionNum;
	Seeds m_seeds;
	Clusters m_result;
};
}
#endif KMEAN_ALGORITHM_H
#ifndef KD_TREE_NANO_FLANN
#define KD_TREE_NANO_FLANN
#include "IAlgorithm.h"
#include "FileUtility.h"

class PointCloudNode;
class PrimitiveNode;
class KDTreeNanoFlann : public IAlgorithm
{
public:
	class Impl
	{
	public:
		Impl() {};
		~Impl() {};
	private:
	};

	KDTreeNanoFlann(PointCloudNode* pointCloud, int dimension);
	~KDTreeNanoFlann();

	virtual ALGORITHM_TYPE GetType() { return ALGORITHM_KDTREE; };
	virtual void Execute();
	virtual void ShowUI();
	Vector<int> GetRadiusNeighbor(const vec3& query, float rad);
private:
	struct UI
	{
		UI() : slider(0)
		{
		}

		int slider;
	};

	UI m_ui;
	PointCloudNode* m_pPointCloud;
	KDTreeNanoFlann::Impl* m_pImpl;
};


#endif 
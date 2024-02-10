#ifndef HARRIS3D_TREE
#define HARRIS3D_TREE
#include "IAlgorithm.h"
class PointCloudNode;
class Harris3D : public IAlgorithm
{
public:
	Harris3D(PointCloudNode* pointCloud);
	~Harris3D();
	virtual ALGORITHM_TYPE GetType() { return ALGORITHM_HARRIS3D; };

	virtual void Execute();
	virtual void ShowUI();
private:
	Vector<float> m_result;
	PointCloudNode* m_pPointCloud;
	float m_max;
	float m_min;
};

#endif HARRIS3D_TREE
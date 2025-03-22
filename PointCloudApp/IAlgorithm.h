#ifndef IALGORITHM_H
#define IALGORITHM_H
#include "RenderNode.h"
namespace KI
{

enum ALGORITHM_TYPE
{
	ALGORITHM_KDTREE,
	ALGORITHM_HARRIS3D,
	ALGORITHM_ALPHASHAPE,
	ALGORITHM_DELAUNAY,
	ALGORITHM_SHAPE_DIAMETER,
	ALGORITHM_SIGNED_DISTANCE_FIELD,
	ALGORITHM_VOXELIZER,
	ALGORITHM_BVH,
};
class IAlgorithm
{
public:
	IAlgorithm() {};
	virtual ~IAlgorithm() {};
	virtual ALGORITHM_TYPE GetType() = 0;
	virtual void Execute() {};
	virtual void ShowUI(UIContext& ui) = 0;
private:

};
}

#endif
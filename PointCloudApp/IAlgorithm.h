#ifndef IALGORITHM_H
#define IALGORITHM_H
namespace KI
{
struct UIContext;
class RenderNode;
enum ALGORITHM_TYPE
{
	ALGORITHM_KDTREE,
	ALGORITHM_HARRIS3D,
	ALGORITHM_ALPHASHAPE,
	ALGORITHM_DELAUNAY,
	ALGORITHM_DELAUNAY_3D,
	ALGORITHM_SHAPE_DIAMETER,
	ALGORITHM_SIGNED_DISTANCE_FIELD,
	ALGORITHM_VOXELIZER,
	ALGORITHM_MINIMUM_SPANNING_TREE,
	ALGORITHM_BVH,
	ALGORITHM_CROSS_SECTION_LINE,
};
class IAlgorithm
{
public:
	IAlgorithm() {};
	virtual ~IAlgorithm() {};
	virtual ALGORITHM_TYPE GetType() = 0;
	virtual void Execute() {};
	virtual void ShowUI(RenderNode* pNode, UIContext& ui) {};
private:

};
}

#endif
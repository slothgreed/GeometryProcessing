#ifndef KI_BVH
#define KI_BVH
#include "KIMath.h"
#include "IAlgorithm.h"
#include "GLBuffer.h"
#include "GeometryUtility.h"
namespace KI
{
class HalfEdgeNode;
class BVH : public IAlgorithm
{
public:

	struct Node
	{
		Node()
			: left(-1)
			, right(-1)
			, parent(-1)
			, triangleIndex(-1)
			, morton(0)
		{
		}

		Node(const BDB& _bdb, unsigned int _morton, int _triIndex)
			: minBox(_bdb.Min())
			, maxBox(_bdb.Max())
			, morton(_morton)
			, triangleIndex(_triIndex)
			, left(-1)
			, right(-1)
			, parent(-1)

		{

		}
		Vector3 minBox;
		Vector3 maxBox;
		unsigned int morton;
		int left;
		int right;
		int parent;
		int triangleIndex;
	};


	BVH(HalfEdgeNode* pNode):m_pHalfEdge(pNode)  {}
	virtual ~BVH() {};
	virtual ALGORITHM_TYPE GetType() { return ALGORITHM_BVH; }
	virtual void Execute();
	virtual void ShowUI();

	struct IntersectResult
	{
		IntersectResult()
			: faceIndex(-1)
			, distance(0)
		{
		}

		IntersectResult(int _f, const Vector3& pos, float _d)
			: faceIndex(_f)
			, position(pos)
			, distance(_d)
		{
		}

		bool IsSuccess() const { return faceIndex != -1; }
		Vector3 position;
		int faceIndex;
		float distance;
	};


	BVH::IntersectResult IntersectMinFace(const Ray& ray) const;

	Vector<BVH::IntersectResult> IntersectFace(const Ray& ray) const;
	void DeleteUINode();

private:
	void CreateGPUBuffer();
	void CountLeafNodes(int nodeIndex, int& leafNum);
	void DebugForAllLeaf();

	struct GPU
	{
		Unique<GLBuffer> pBuffer;
	};


	struct UI
	{
		UI()
			:showLevel(-1)
		{
		}

		int showLevel;
	};

	UI m_ui;
	GPU m_gpu;
	HalfEdgeNode* m_pHalfEdge;
	Vector<std::pair<int, int>> m_levelRange;
	Vector<Node> m_nodes;
};

}

#endif KI_BVH
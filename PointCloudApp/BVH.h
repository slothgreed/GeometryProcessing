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
		Node() : Node(BDB(), -1, -1) {}
		Node(const BDB& _bdb, unsigned int _morton, int _triIndex)
		{
			SetMin(_bdb.Min());
			SetMax(_bdb.Max());
			SetMorton(_morton);
			SetTriangle(_triIndex);
			SetLeft(-1); SetRight(-1); SetParent(-1);
		}

		Vector3 MinBox() const { return Vector3(minBox.x, minBox.y, minBox.z); }
		Vector3 MaxBox() const { return Vector3(maxBox.x, maxBox.y, maxBox.z); }
		int Left() const { return minBox.w; }
		int Right() const { return maxBox.w; }
		int Parent() const { return mix.x; }
		int Triangle() const { return mix.z; }
		void SetLeft(int value) { minBox.w = (float)value; }
		void SetRight(int value) { maxBox.w = (float)value; }
		void SetMorton(unsigned int value) { mix.y = value; }
		void SetMin(const Vector3& value) { minBox.x = value.x; minBox.y = value.y; minBox.z = value.z; }
		void SetMax(const Vector3& value) { maxBox.x = value.x; maxBox.y = value.y; maxBox.z = value.z; }
		void SetParent(int value) { mix.x = value; }
		void SetTriangle(int value) { mix.z = value; }
	private:

		Vector4 minBox; // w = left;
		Vector4 maxBox; // w = right;
		Vector4u mix; // x = parent; y = morton; z = triangleIndex; 
	};


	BVH(HalfEdgeNode* pNode):m_pHalfEdge(pNode)  {}
	virtual ~BVH() {};
	virtual ALGORITHM_TYPE GetType() { return ALGORITHM_BVH; }
	virtual void Execute();
	virtual void ShowUI(RenderNode* pNode, UIContext& ui);

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


	BVH::IntersectResult CalcMinDistance(const Vector3& pos) const;
	BVH::IntersectResult IntersectMinFace(const Ray& ray) const;
	Vector<BVH::IntersectResult> IntersectFace(const Ray& ray) const;
	void DeleteUINode();
	int GetMaxLevel() const { return m_levelRange.size(); }
	const Vector<BVH::Node>& GetNode() const { return m_nodes; }
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
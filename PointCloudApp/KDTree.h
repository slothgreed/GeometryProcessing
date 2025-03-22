#ifndef KD_TREE
#define KD_TREE
#include "IAlgorithm.h"
#include "FileUtility.h"
namespace KI
{
class PointCloudNode;
class PrimitiveNode;
class BDB;
class KDTree : public IAlgorithm
{
public:

	virtual ALGORITHM_TYPE GetType() { return ALGORITHM_KDTREE; };

	enum class Axis { X, Y, Z };
	KDTree(PointCloudNode* pointCloud, int dimension);
	~KDTree();

	virtual void Execute();
	bool Executed() { return m_index.size() != 0; }
	Vector<Vector4> CreateLevelColor(int target);
	virtual void ShowUI(UIContext& ui);
private:

	struct Node
	{
		Node* pLeft;
		Node* pRight;
		int index;
		Node();
		~Node();
		const Vector3& GetPosition(PointCloudNode* pPointCloud);
	};

	struct ResultNearest
	{
		ResultNearest()
			: minDist(1000000000000.0f)
			, pMin(nullptr), pArea(nullptr)
		{
		}
		float minDist;
		Node* pMin;
		Node* pArea;
	};

	void FindNearest(Node* pNode, int depth, const Vector3& target, ResultNearest& result);
	Node* FindNode(Node* pNode, int depth, const Vector3& target);
	void CreateLevelColor(Node* pNode, Vector<Vector4>& color, int depth, int target);
	void SetLevelColor(Node* pNode, Vector<Vector4>& color, const Vector4& col, int depth);
	Node* Build(int left, int right, int depth);
	void Delete();
	Shared<PrimitiveNode> CreatePartition2D(const String& name, int maxDepth);
	void CreatePartition2D(Node* pNode, int depth, int maxDepth, const BDB& bdb, Vector<Vector3>& position, Vector<unsigned int>& indexes);
	Vector<int> m_index;
	PointCloudNode* m_pPointCloud;
	Node* m_root;
	int m_dimension;
	struct UI
	{
		UI() :depth(0), showLine(false) {}
		int depth;
		bool showLine;
	};

	UI m_ui;
};
}

#endif KD_TREE
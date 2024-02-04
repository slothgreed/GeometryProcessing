#ifndef KD_TREE
#define KD_TREE
#include "IAlgorithm.h"
#include "FileUtility.h"

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
	std::vector<vec3> CreateLevelColor(int target);
	virtual void ShowUI();
private:

	struct Node
	{
		Node* pLeft;
		Node* pRight;
		int index;
		Node();
		~Node();
		const vec3& GetPosition(PointCloudNode* pPointCloud);
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

	void FindNearest(Node* pNode, int depth, const vec3& target, ResultNearest& result);
	Node* FindNode(Node* pNode, int depth, const vec3& target);
	void CreateLevelColor(Node* pNode, std::vector<vec3>& color, int depth, int target);
	void SetLevelColor(Node* pNode, std::vector<vec3>& color, const vec3& col, int depth);
	Node* Build(int left, int right, int depth);
	void Delete();
	shared_ptr<PrimitiveNode> CreatePartition2D(const string& name, int maxDepth);
	void CreatePartition2D(Node* pNode, int depth, int maxDepth, const BDB& bdb, std::vector<vec3>& position, std::vector<unsigned int>& indexes);
	std::vector<int> m_index;
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

#endif KD_TREE
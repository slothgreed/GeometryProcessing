#ifndef KD_TREE
#define KD_TREE
#include "IAlgorithm.h"
#include "FileUtility.h"

class PointCloudNode;
class KDTree : public IAlgorithm
{
public:

	virtual ALGORITHM_TYPE GetType() { return ALGORITHM_KDTREE; };

	enum class Axis { X, Y, Z };
	KDTree(const std::shared_ptr<PointCloudNode>& pointCloud, int dimension);
	~KDTree();

	virtual void Execute();
	bool Executed() { return m_index.size() != 0; }
	std::vector<vec3> CreateColor(int index, float dist);
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
	};

	Node* FindNode(Node* pNode, int index, int depth);
	void CreateLevelColor(Node* pNode, std::vector<vec3>& color, int depth, int target);
	void SetLevelColor(Node* pNode, std::vector<vec3>& color, const vec3& col, int depth);
	Node* Build(int left, int right, int depth);
	void Delete();
	void CreatePartition2D();
	std::vector<int> m_index;
	std::shared_ptr<PointCloudNode> m_pPointCloud;
	Node* m_root;
	int m_dimension;
	struct UI
	{
		int depth;
	};

	UI m_ui;
};

#endif KD_TREE
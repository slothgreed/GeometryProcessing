#include "KDTree.h"
#include "PointCloud.h"
#include "Utility.h"
#include "FileUtility.h"
#include "PointCloudNode.h"
#include <algorithm>
#include <string>
class PointComparator
{
public:

	PointComparator(PointCloud* pointCloud, KDTree::Axis axis)
	: m_pointCloud(pointCloud)
	, m_axis(axis){};
	~PointComparator() {};

	bool operator()(int lhs, int rhs)
	{
		if (m_axis == KDTree::Axis::X) {
			return m_pointCloud->Position()[lhs].x < m_pointCloud->Position()[rhs].x;
		} else if (m_axis == KDTree::Axis::Y) {
			return m_pointCloud->Position()[lhs].y < m_pointCloud->Position()[rhs].y;
		} else if (m_axis == KDTree::Axis::Z) {
			return m_pointCloud->Position()[lhs].z < m_pointCloud->Position()[rhs].z;
		}

		return false;
	}
private:
	PointCloud* m_pointCloud;
	KDTree::Axis m_axis;
};


KDTree::Node::Node()
	:pLeft(nullptr), pRight(nullptr), index(-1)
{

}

KDTree::Node::~Node()
{
	if (pLeft) { delete pLeft; pLeft = nullptr; }
	if (pRight) { delete pRight; pRight = nullptr; }
}

KDTree::KDTree(const std::shared_ptr<PointCloudNode>& pointCloud, int dimension)
	: m_pPointCloud(pointCloud)
	, m_root(nullptr)
	, m_dimension(dimension)
{
}


void KDTree::ShowUI()
{
	if (ImGui::Button("KDTree")) {
		if (!Executed()) {
			Execute();
		}
	}

	if (!Executed()) {
		return;
	}

	if (ImGui::SliderInt("Depth", &m_ui.depth, 0, std::log(m_pPointCloud->GetData()->Position().size()))) {
		m_pPointCloud->GetData()->SetColor(CreateLevelColor(m_ui.depth));
		m_pPointCloud->UpdateData();
	}

	if (ImGui::Button("CreateLine")) {

	}
}
KDTree::~KDTree()
{
	if (m_root) {
		delete m_root;
		m_root = nullptr;
	}
}


void KDTree::Delete()
{
}


void KDTree::Execute()
{
	m_index.resize(m_pPointCloud->GetData()->Position().size());
	for (int i = 0; i < m_index.size(); i++) {
		m_index[i] = i;
	}

	m_root = Build(0, m_index.size(), 0);
}


KDTree::Node* KDTree::Build(int left, int right, int depth)
{
	if (left >= right) { return nullptr; }
	int middle = (left + right) >> 1;
	int axis = depth % 3;
	if (axis == 0) {
		std::sort(
			m_index.begin() + left, 
			m_index.begin() + right,
			PointComparator(m_pPointCloud->GetData().get(), Axis::X));
	} else if(axis == 1){
		std::sort(
			m_index.begin() + left,
			m_index.begin() + right,
			PointComparator(m_pPointCloud->GetData().get(), Axis::Y));
	} else if (axis == 2) {
		std::sort(
			m_index.begin() + left,
			m_index.begin() + right,
			PointComparator(m_pPointCloud->GetData().get(), Axis::Z));
	}

	auto pNode = new Node();
	pNode->pLeft = Build(left, middle, depth + 1);
	pNode->pRight = Build(middle + 1, right, depth + 1);
	pNode->index = middle;
	return pNode;
}

KDTree::Node* KDTree::FindNode(Node* pNode, int index, int depth)
{
	if (pNode->index == index) { return pNode; }
	auto axis = depth % 3;
	if (axis == 0) {
		if (pNode->pLeft && m_pPointCloud->GetData()->Position()[pNode->index].x < m_pPointCloud->GetData()->Position()[index].x) {
			return FindNode(pNode->pLeft, index, depth + 1);
		} else if(pNode->pRight) {
			return FindNode(pNode->pRight, index, depth + 1);
		}
	} else if (axis == 1) {
		if (pNode->pLeft && m_pPointCloud->GetData()->Position()[pNode->index].y < m_pPointCloud->GetData()->Position()[index].y) {
			return FindNode(pNode->pLeft, index, depth + 1);
		} else if (pNode->pRight) {
			return FindNode(pNode->pRight, index, depth + 1);
		}
	} else if (axis == 2) {
		if (pNode->pLeft && m_pPointCloud->GetData()->Position()[pNode->index].z < m_pPointCloud->GetData()->Position()[index].z) {
			return FindNode(pNode->pLeft, index, depth + 1);
		} else if (pNode->pRight) {
			return FindNode(pNode->pRight, index, depth + 1);
		}
	}

	assert(0);
	return nullptr;
}

void KDTree::CreatePartition2D()
{

}
void KDTree::SetLevelColor(Node* pNode, std::vector<vec3>& color, const vec3& col, int depth)
{
	if (!pNode) { return; }
	color[m_index[pNode->index]] = col;
	SetLevelColor(pNode->pLeft, color, col, depth + 1);
	SetLevelColor(pNode->pRight, color, col, depth + 1);
}
void KDTree::CreateLevelColor(Node* pNode, std::vector<vec3>& color, int depth, int target)
{
	if (!pNode) { return; }
	if (depth == target) {
		SetLevelColor(pNode, color, ColorUtility::CreateRandom(), depth);
	}

	CreateLevelColor(pNode->pLeft, color, depth + 1, target);
	CreateLevelColor(pNode->pRight, color, depth + 1, target);
}

std::vector<vec3> KDTree::CreateLevelColor(int target)
{
	std::vector<vec3> color(m_index.size());
	CreateLevelColor(m_root, color, 0, target);
	return color;
}
std::vector<vec3> KDTree::CreateColor(int index, float dist)
{
	std::vector<vec3> color(m_index.size());

	std::vector<int> foundIndex;
	const auto& base = m_pPointCloud->GetData()->Position()[index];
	for (int i = 0; i < m_pPointCloud->GetData()->Position().size(); i++) {
		auto node = FindNode(m_root, i, 0);
		if (!node) {
			int a = 0;
		}
	}
	auto node = FindNode(m_root, 2, 0);
	if (node) {
		for (int i = 0; i < color.size(); i++) {
			color[i] = vec3(1, 0, 0);
		}
	} else {
		for (int i = 0; i < color.size(); i++) {
			color[i] = vec3(1, 1, 0);
		}
	}

	return color;
}


#include "KDTree.h"
#include "PointCloud.h"
#include "Utility.h"
#include "FileUtility.h"
#include "PointCloudNode.h"
#include "PrimitiveNode.h"
#include "Primitives.h"

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

KDTree::KDTree(PointCloudNode* pointCloud, int dimension)
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

	if (ImGui::Checkbox("ShowLine", &m_ui.showLine)) {
		if (m_dimension != 2) { m_ui.showLine = false; } // ”ñ‘Î‰ž
		if (m_ui.showLine) {
			m_pPointCloud->SetNode(CreatePartition2D("Partition2D", m_ui.depth));
		} else {
			m_pPointCloud->RemoveNode("Partition2D");
		}
	}

	if (ImGui::Button("Find")) {

		int i = 4;
		//for (int i = 0; i < 20; i++)
		{
			//vec3 target = Random::Vec3(0, 100); target.z = 0.0f;
			//vec3 target = Random::Vec3(vec3(0,90,0), vec3(30,100,0)); target.z = 0.0f;
			vec3 target = vec3(23, 87, 0);

			ResultNearest result;
			FindNearest(m_root, 0, target, result);
			const auto& ret = m_pPointCloud->GetData()->Position()[result.pMin->index];
			auto pPoint = std::make_shared<Primitive>();
			std::vector<vec3> pos;
			pos.push_back(target);
			pos.push_back(ret);
			pPoint->SetPosition(std::move(pos));
			pPoint->SetType(GL_LINES);
			m_pPointCloud->SetNode(std::make_shared<PrimitiveNode>("Line" + std::to_string(i), pPoint, ColorUtility::CreatePrimary(i)));

			std::shared_ptr<Primitive> pCircle = std::make_shared<Circle>(glm::length(target - ret), target);
			m_pPointCloud->SetNode(std::make_shared<PrimitiveNode>("Range" + std::to_string(i), pCircle, ColorUtility::CreatePrimary(i)));
			Printf::Vec3(std::to_string(i), target);
		}
	}
	if (ImGui::SliderInt("Depth", &m_ui.depth, 0, std::log2(m_pPointCloud->GetData()->Position().size()) + 1)) {
		m_pPointCloud->GetData()->SetColor(CreateLevelColor(m_ui.depth));
		m_pPointCloud->UpdateData();
		if (m_ui.showLine) {
			m_pPointCloud->SetNode(CreatePartition2D("Partition2D", m_ui.depth));
		}
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
	int axis = depth % m_dimension;
	if (axis == (int)Axis::X) {
		std::sort(
			m_index.begin() + left, 
			m_index.begin() + right,
			PointComparator(m_pPointCloud->GetData().get(), Axis::X));
	} else if(axis == (int)Axis::Y){
		std::sort(
			m_index.begin() + left,
			m_index.begin() + right,
			PointComparator(m_pPointCloud->GetData().get(), Axis::Y));
	} else if (axis == (int)Axis::Z) {
		std::sort(
			m_index.begin() + left,
			m_index.begin() + right,
			PointComparator(m_pPointCloud->GetData().get(), Axis::Z));
	} else {
		assert(0);
	}

	auto pNode = new Node();
	pNode->index = m_index[middle];
	pNode->pLeft = Build(left, middle, depth + 1);
	pNode->pRight = Build(middle + 1, right, depth + 1);
	return pNode;
}

const vec3& KDTree::Node::GetPosition(PointCloudNode* pPointCloud)
{
	return pPointCloud->GetData()->Position()[index];
}

void KDTree::FindNearest(Node* pNode, int depth, const vec3& target, ResultNearest& result)
{
	const auto& pos = pNode->GetPosition(m_pPointCloud);

	float dist = glm::length(pos - target);
	if (dist < result.minDist) {
		result.minDist = dist;
		result.pMin = pNode;
	}

	auto axis = depth % m_dimension;
	if (pNode->pLeft && target[axis] <= pos[axis]) {
		FindNearest(pNode->pLeft, depth + 1, target, result);
		if (pNode->pRight) {
			FindNearest(pNode->pRight, depth + 1, target, result);
		}
	} else if (pNode->pRight) {
		FindNearest(pNode->pRight, depth + 1, target, result);
		if (pNode->pLeft) {
			FindNearest(pNode->pLeft, depth + 1, target, result);
		}
	}
}
KDTree::Node* KDTree::FindNode(Node* pNode, int depth, const vec3& target)
{
	const auto& pos = pNode->GetPosition(m_pPointCloud);
	if (pos == target) { return pNode; }
	auto axis = depth % m_dimension;
	if (pNode->pLeft && target[axis] < pos[axis]) {
		return FindNode(pNode->pLeft, depth + 1, target);
	} else if (pNode->pRight) {
		return FindNode(pNode->pRight, depth + 1, target);
	}

	assert(0);
	return nullptr;
}

void AddPartition2DPosition(
	std::vector<glm::vec3>& position, 
	std::vector<int>& index,
	const BDB& bdb)
{
	int current = position.size();
	position.push_back(bdb.Min());
	position.push_back(vec3(bdb.Max().x, bdb.Min().y, bdb.Min().z));
	position.push_back(bdb.Max());
	position.push_back(vec3(bdb.Min().x, bdb.Max().y, bdb.Min().z));
	index.push_back(current++);	index.push_back(current);
	index.push_back(current++);	index.push_back(current);
	index.push_back(current++);	index.push_back(current);
	index.push_back(current++);	index.push_back(position.size() - 4);
}
void KDTree::CreatePartition2D(Node* pNode, int depth, int maxDepth, const BDB& bdb, std::vector<vec3>& position, std::vector<int>& indexes)
{
	if (!pNode) { return; } if (maxDepth == depth) { return; }
	const auto& pos = pNode->GetPosition(m_pPointCloud);
	BDB leftBDB;
	BDB rightBDB;
	if (depth % 2 == (int)Axis::X) {
		rightBDB = BDB(vec3(pos.x, bdb.Min().y, 0.0f), bdb.Max());
		leftBDB = BDB(bdb.Min(), vec3(pos.x, bdb.Max().y, 0.0f));
	} else if (depth % 2 == (int)Axis::Y) {
		rightBDB = BDB(vec3(bdb.Min().x, pos.y, 0.0f), bdb.Max());
		leftBDB = BDB(bdb.Min(), vec3(bdb.Max().x, pos.y, 0.0f));
	}

	//if (maxDepth - 1 == depth) 
	{
		AddPartition2DPosition(position, indexes, leftBDB);
		AddPartition2DPosition(position, indexes, rightBDB);
	}

	CreatePartition2D(pNode->pLeft, depth + 1, maxDepth, leftBDB, position, indexes);
	CreatePartition2D(pNode->pRight, depth + 1, maxDepth, rightBDB, position, indexes);
}
shared_ptr<PrimitiveNode> KDTree::CreatePartition2D(const string& name, int maxDepth)
{
	auto pPrimitive = std::make_shared<Primitive>();
	std::vector<glm::vec3> position;
	std::vector<int> indexes;
	const auto& bdb = m_pPointCloud->GetData()->GetBDB();

	CreatePartition2D(m_root, 0, maxDepth, bdb, position, indexes);

	pPrimitive->SetPosition(std::move(position));
	pPrimitive->SetIndex(std::move(indexes));
	pPrimitive->SetType(GL_LINES);
	return make_shared<PrimitiveNode>(name, pPrimitive, vec3(1, 0, 0));
}
void KDTree::SetLevelColor(Node* pNode, std::vector<vec3>& color, const vec3& col, int depth)
{
	if (!pNode) { return; }
	color[pNode->index] = col;
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


#include "KDTree.h"
#include "PointCloud.h"
#include "Utility.h"
#include "PointCloudNode.h"
#include "PrimitiveNode.h"
#include "Primitives.h"
namespace KI
{
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


void KDTree::ShowUI(UIContext& ui)
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
			m_pPointCloud->AddNode(CreatePartition2D("Partition2D", m_ui.depth));
		} else {
			m_pPointCloud->RemoveNode("Partition2D");
		}
	}

	if (ImGui::Button("Find")) {

		int i = 4;
		//for (int i = 0; i < 20; i++)
		{
			//Vector3 target = Random::Vec3(0, 100); target.z = 0.0f;
			//Vector3 target = Random::Vec3(Vector3(0,90,0), Vector3(30,100,0)); target.z = 0.0f;
			Vector3 target = Vector3(23, 87, 0);

			ResultNearest result;
			FindNearest(m_root, 0, target, result);
			const auto& ret = m_pPointCloud->GetData()->Position()[result.pMin->index];
			auto pPoint = std::make_shared<Primitive>();
			std::vector<Vector3> pos;
			pos.push_back(target);
			pos.push_back(ret);
			pPoint->SetPosition(std::move(pos));
			pPoint->SetType(GL_LINES);
			m_pPointCloud->AddNode(std::make_shared<PrimitiveNode>("Line" + std::to_string(i), pPoint, ColorUtility::CreatePrimary(i)));

			Shared<Primitive> pCircle = std::make_shared<Circle>(glm::length(target - ret), target);
			m_pPointCloud->AddNode(std::make_shared<PrimitiveNode>("Range" + std::to_string(i), pCircle, ColorUtility::CreatePrimary(i)));
			DebugPrintf::Vec3(std::to_string(i), target);
			DebugPrintf::NewLine();
		}
	}
	if (ImGui::SliderInt("Depth", &m_ui.depth, 0, std::log2(m_pPointCloud->GetData()->Position().size()) + 1)) {
		m_pPointCloud->GetData()->SetColor(CreateLevelColor(m_ui.depth));
		m_pPointCloud->UpdateData();
		if (m_ui.showLine) {
			m_pPointCloud->AddNode(CreatePartition2D("Partition2D", m_ui.depth));
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

const Vector3& KDTree::Node::GetPosition(PointCloudNode* pPointCloud)
{
	return pPointCloud->GetData()->Position()[index];
}

void KDTree::FindNearest(Node* pNode, int depth, const Vector3& target, ResultNearest& result)
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
KDTree::Node* KDTree::FindNode(Node* pNode, int depth, const Vector3& target)
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
	Vector<Vector3>& position, 
	Vector<unsigned int>& index,
	const BDB& bdb)
{
	int current = position.size();
	position.push_back(bdb.Min());
	position.push_back(Vector3(bdb.Max().x, bdb.Min().y, bdb.Min().z));
	position.push_back(bdb.Max());
	position.push_back(Vector3(bdb.Min().x, bdb.Max().y, bdb.Min().z));
	index.push_back(current++);	index.push_back(current);
	index.push_back(current++);	index.push_back(current);
	index.push_back(current++);	index.push_back(current);
	index.push_back(current++);	index.push_back(position.size() - 4);
}
void KDTree::CreatePartition2D(Node* pNode, int depth, int maxDepth, const BDB& bdb, Vector<Vector3>& position, Vector<unsigned int>& indexes)
{
	if (!pNode) { return; } if (maxDepth == depth) { return; }
	const auto& pos = pNode->GetPosition(m_pPointCloud);
	BDB leftBDB;
	BDB rightBDB;
	if (depth % 2 == (int)Axis::X) {
		rightBDB = BDB(Vector3(pos.x, bdb.Min().y, 0.0f), bdb.Max());
		leftBDB = BDB(bdb.Min(), Vector3(pos.x, bdb.Max().y, 0.0f));
	} else if (depth % 2 == (int)Axis::Y) {
		rightBDB = BDB(Vector3(bdb.Min().x, pos.y, 0.0f), bdb.Max());
		leftBDB = BDB(bdb.Min(), Vector3(bdb.Max().x, pos.y, 0.0f));
	}

	//if (maxDepth - 1 == depth) 
	{
		AddPartition2DPosition(position, indexes, leftBDB);
		AddPartition2DPosition(position, indexes, rightBDB);
	}

	CreatePartition2D(pNode->pLeft, depth + 1, maxDepth, leftBDB, position, indexes);
	CreatePartition2D(pNode->pRight, depth + 1, maxDepth, rightBDB, position, indexes);
}
Shared<PrimitiveNode> KDTree::CreatePartition2D(const String& name, int maxDepth)
{
	auto pPrimitive = std::make_shared<Primitive>();
	Vector<Vector3> position;
	Vector<unsigned int> indexes;
	const auto& bdb = m_pPointCloud->GetData()->GetBDB();

	CreatePartition2D(m_root, 0, maxDepth, bdb, position, indexes);

	pPrimitive->SetPosition(std::move(position));
	pPrimitive->SetIndex(std::move(indexes));
	pPrimitive->SetType(GL_LINES);
	return std::make_shared<PrimitiveNode>(name, pPrimitive, Vector3(1, 0, 0));
}
void KDTree::SetLevelColor(Node* pNode, Vector<Vector4>& color, const Vector4& col, int depth)
{
	if (!pNode) { return; }
	color[pNode->index] = col;
	SetLevelColor(pNode->pLeft, color, col, depth + 1);
	SetLevelColor(pNode->pRight, color, col, depth + 1);
}
void KDTree::CreateLevelColor(Node* pNode, Vector<Vector4>& color, int depth, int target)
{
	if (!pNode) { return; }
	if (depth == target) {
		SetLevelColor(pNode, color, ColorUtility::CreateRandom4(), depth);
	}

	CreateLevelColor(pNode->pLeft, color, depth + 1, target);
	CreateLevelColor(pNode->pRight, color, depth + 1, target);
}

Vector<Vector4> KDTree::CreateLevelColor(int target)
{
	Vector<Vector4> color(m_index.size());
	CreateLevelColor(m_root, color, 0, target);
	return color;
}

}
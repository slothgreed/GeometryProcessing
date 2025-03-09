#include "BVH.h"
#include "HalfEdgeNode.h"
#include "IShader.h"
#include "Utility.h"
#include "PrimitiveNode.h"
#include "Primitives.h"
namespace KI
{

void BVH::Execute()
{
	if (m_nodes.size() != 0) { return; }
	m_nodes.clear();
	m_levelRange.clear();
	const auto& morton = m_pHalfEdge->GetMorton();
	m_nodes.reserve(morton.Get().size() * 2 - 1);
	for (int i = 0; i < morton.Get().size(); i++) {

		m_nodes.push_back(
			Node(morton.Get()[i].box, morton.Get()[i].morton, morton.Get()[i].triangleIndex));
	}

	int leaves = morton.Get().size();
	int offset = 0;
	m_levelRange.push_back(std::pair<int, int>(offset, leaves));

	while (leaves > 1) {
		int nextOffset = m_nodes.size();
		for (int i = 0; i < leaves - 1; i += 2) {
			int parentIndex = m_nodes.size();
			int left = offset + i;
			int right = offset + i + 1;

			Node node;
			node.left = left;
			node.right = right;
			node.box.Add(m_nodes[left].box);
			node.box.Add(m_nodes[right].box);
			m_nodes.push_back(std::move(node));

			m_nodes[left].parent = parentIndex;
			m_nodes[right].parent = parentIndex;
		}

		// ��̏ꍇ�̏���
		if (leaves % 2 == 1) {
			int lastIndex = offset + leaves - 1;
			m_nodes.push_back(m_nodes[lastIndex]); // �R�s�[�ł͂Ȃ��P�ƃm�[�h
		}

		offset = nextOffset;
		leaves = (leaves + 1) / 2;
		m_levelRange.push_back(std::pair<int, int>(offset, m_nodes.size()));
	}

	/*
	int leaves = (morton.Get().size() + 1) / 2;
	int offset = 0;
	m_levelRange.push_back(std::pair<int, int>(offset, m_nodes.size()));
	while (leaves > 1) {
		for (int i = 0; i < leaves; i += 2) {
			int parentIndex = m_nodes.size();
			int left = offset + i;
			int right = offset + i + 1;

			Node node;
			node.left = left;
			node.right = right;
			node.box.Add(m_nodes[left].box);
			node.box.Add(m_nodes[right].box);
			m_nodes.push_back(std::move(node));

			m_nodes[left].parent = parentIndex;
			m_nodes[right].parent = parentIndex;
		}

		offset = m_levelRange[m_levelRange.size() - 1].second;
		if (leaves % 2 == 1) {
			m_nodes.push_back(m_nodes[offset - 1]);
		}

		leaves = (leaves + 1) / 2;
		m_levelRange.push_back(std::pair<int, int>(offset, m_nodes.size()));
	}
	*/

	/* TestCode
	{


		struct Node
		{
			unsigned int morton;
			int left;
			int right;
			int parent;
		};

		std::vector<Node> nodes = {
		{0, -1, -1, -1}, {1, -1, -1, -1}, {2, -1, -1, -1}, {3, -1, -1, -1},
		{4, -1, -1, -1}, {5, -1, -1, -1}, {6, -1, -1, -1}, {7, -1, -1, -1}
		};

		// ���[�g���R�[�h���ɕ���ł���̂Ń\�[�g�͕s�v�����A�����I�Ɋm�F
		std::sort(nodes.begin(), nodes.end(), [](const Node& a, const Node& b)
		{
			return a.morton < b.morton;
		});

		int numLeaves = nodes.size();
		std::vector<Node> bvhNodes;
		bvhNodes.reserve(2 * numLeaves - 1);

		// ���[�t�m�[�h���R�s�[
		for (int i = 0; i < numLeaves; i++) {
			bvhNodes.push_back(nodes[i]);
		}

		int offset = 0;
		while (numLeaves > 1) {
			int newSize = 0;
			for (int i = 0; i < numLeaves - 1; i += 2) {
				int left = offset + i;
				int right = offset + i + 1;
				int parentIndex = bvhNodes.size();

				bvhNodes.push_back({ 0, left, right, -1 });
				bvhNodes[left].parent = parentIndex;
				bvhNodes[right].parent = parentIndex;
				newSize++;
			}

			// �]�����m�[�h�i��������ꍇ�j�����̂܂ܒǉ�
			if (numLeaves % 2 == 1) {
				bvhNodes.push_back(bvhNodes[offset + numLeaves - 1]);
			}

			offset += numLeaves;
			numLeaves = (numLeaves + 1) / 2;
		}
		nodes = bvhNodes;

		// �����ɔz�u�������ʂ��o��
		printf("Sorted Morton Codes:\n");
		for (const auto& node : nodes) {
			printf("Morton: %06b, Left: %d, Right: %d\n", node.morton, node.left, node.right);
		}

	}
	*/
	DebugForAllLeaf();
}

HalfEdgeStruct::Face GetFace(const HalfEdgeNode* pHalfEdge, int triangleIndex)
{
	return pHalfEdge->GetData()->GetFace(triangleIndex);
}

BVH::IntersectResult BVH::IntersectMinFace(const Ray& ray) const
{
	auto intersects = IntersectFace(ray);
	if (intersects.size() == 0) {
		return BVH::IntersectResult();
	}

	BVH::IntersectResult minResult = intersects[0];
	
	for (int i = 1; i < intersects.size(); i++) {
		if (minResult.distance > intersects[i].distance) {
			minResult = intersects[i];
		}
	}

	return minResult;
}

Vector<BVH::IntersectResult> BVH::IntersectFace(const Ray& ray) const
{
	if (m_nodes.empty()) { return Vector<BVH::IntersectResult>(); }

	std::vector<int> stack;
	stack.push_back(m_nodes.size() - 1); // ���[�g�m�[�h����J�n
	Vector<BVH::IntersectResult> hit;
	while (!stack.empty()) {
		int nodeIndex = stack.back();
		stack.pop_back();

		const Node& node = m_nodes[nodeIndex];

		auto intersect = ray.Intersect(node.box);
		// AABB�ƌ������Ȃ��ꍇ�̓X�L�b�v
		if (!intersect.success) { continue; }

		if (node.left == -1 && node.right == -1) {
			// ���[�t�m�[�h�Ȃ�������X�g�ɒǉ�
			auto face = GetFace(m_pHalfEdge, node.triangleIndex);
			auto triIntersect = ray.Intersect(face.pos0, face.pos1, face.pos2, false);
			if (triIntersect.success) {
				hit.push_back(BVH::IntersectResult(node.triangleIndex, triIntersect.position, triIntersect.distance));
			}
		} else {
			// �����m�[�h�Ȃ�q�m�[�h���X�^�b�N�ɒǉ��i�E���ɓ����ƍ�����ɏ��������j
			if (node.right != -1) stack.push_back(node.right);
			if (node.left != -1) stack.push_back(node.left);
		}
	}

	return hit;
}

void BVH::CountLeafNodes(int nodeIndex, int& leafNum)
{
	if (nodeIndex < 0 || nodeIndex >= m_nodes.size()) {
		std::cerr << "Invalid node index: " << nodeIndex << std::endl;
		return;
	}

	const Node& node = m_nodes[nodeIndex];

	// ���[�t�m�[�h�Ȃ�o��
	if (node.left == -1 && node.right == -1) {
		leafNum++;
		return;
	}

	// ���̎q�����ǂ�
	if (node.left != -1) {
		CountLeafNodes(node.left, leafNum);
	}

	// �E�̎q�����ǂ�
	if (node.right != -1) {
		CountLeafNodes(node.right, leafNum);
	}
}

void BVH::DebugForAllLeaf()
{
	if (m_nodes.empty()) {
		std::cerr << "BVH is empty!" << std::endl;
		return;
	}

	int leafNum = 0;
	CountLeafNodes(m_nodes.size() - 1,leafNum);
	if (leafNum == m_pHalfEdge->GetMorton().Get().size()) {
		std::cerr << "BVH is Success!" << std::endl;
	} else {
		assert(0);
		std::cerr << "BVH is notSucess!" << std::endl;
	}
}

void BVH::DeleteUINode()
{
	m_pHalfEdge->RemoveNode("BVH");
}
void BVH::ShowUI()
{
	if (ImGui::SliderInt("ShowLevel", &m_ui.showLevel, -1, m_levelRange.size() - 1)) {
		if (m_ui.showLevel == -1) { return; }
		size_t offset = 0;
		Vector<Vector3> position;
		Vector<unsigned int> indexs;
		for (int i = m_levelRange[m_ui.showLevel].first; i < m_levelRange[m_ui.showLevel].second; i++) {
			auto cube = Cube::CreateLine(m_nodes[i].box.Min(), m_nodes[i].box.Max());
			for (int j = 0; j < cube.Position().size(); j++) {
				position.push_back(cube.Position()[j]);
			}
			for (int j = 0; j < cube.Index().size(); j++) {
				indexs.push_back(cube.Index()[j] + offset);
			}

			offset = position.size();
		}

		auto pCube = std::make_shared<Primitive>();
		pCube->SetPosition(std::move(position));
		pCube->SetIndex(std::move(indexs));
		pCube->SetType(GL_LINES);
		auto pNode = std::make_shared<PrimitiveNode>("BVH", pCube, ColorUtility::CreatePrimary(2));
		pNode->SetMatrix(m_pHalfEdge->GetMatrix());
		m_pHalfEdge->AddNode(pNode);
	}
}
}
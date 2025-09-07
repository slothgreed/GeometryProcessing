#include "BVH.h"
#include "HalfEdgeNode.h"
#include "IShader.h"
#include "Utility.h"
#include "PrimitiveNode.h"
#include "Primitives.h"
#include <queue>
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
			node.SetLeft(left);
			node.SetRight(right);
			BDB box;
			box.Add(m_nodes[left].MinBox()); box.Add(m_nodes[left].MaxBox());
			box.Add(m_nodes[right].MinBox());	box.Add(m_nodes[right].MaxBox());
			
			node.SetMin(box.Min()); node.SetMax(box.Max());
			m_nodes.push_back(std::move(node));

			m_nodes[left].SetParent(parentIndex);
			m_nodes[right].SetParent(parentIndex);
		}

		// 奇数個の場合の処理
		if (leaves % 2 == 1) {
			int lastIndex = offset + leaves - 1;
			m_nodes.push_back(m_nodes[lastIndex]); // コピーではなく単独ノード
		}

		offset = nextOffset;
		leaves = (leaves + 1) / 2;
		m_levelRange.push_back(std::pair<int, int>(offset, m_nodes.size()));
	}

	DebugForAllLeaf();
}

HalfEdgeStruct::Face GetFace(const HalfEdgeNode* pHalfEdge, int triangleIndex)
{
	return pHalfEdge->GetData()->GetFace(triangleIndex);
}

BVH::IntersectResult BVH::CalcMinDistance(const Vector3& pos) const
{
	if (m_nodes.empty()) { return BVH::IntersectResult(); }

	auto face = m_pHalfEdge->GetData()->GetFace(0);
	auto pToTri = Intersect::PointToTriangle(pos, face.pos0, face.pos1, face.pos2);

	auto minDist = BVH::IntersectResult(0, pToTri.position, pToTri.distance);
	std::vector<std::pair<int,int>> stack;
	stack.push_back({ m_nodes.size() - 1,0 }); // ルートノードから開始
	// 深さ優先探索
	while (!stack.empty()) {
		auto nodeIndex = stack.back();
		stack.pop_back();

		const Node& node = m_nodes[nodeIndex.first];

		auto intersect = Intersect::PointToBox(pos, BDB(node.MinBox(), node.MaxBox()), false);
		// AABBの最短距離が面との最短距離より大きい場合はスキップ
		if (intersect.distance > minDist.distance) { continue; }
		if (node.Left() == -1 && node.Right() == -1) {
			// リーフノードなら交差リストに追加
			auto face = GetFace(m_pHalfEdge, node.Triangle());
			auto triIntersect = Intersect::PointToTriangle(pos, face.pos0, face.pos1, face.pos2);
			if (triIntersect.distance < minDist.distance) {
				minDist = BVH::IntersectResult(node.Triangle(), triIntersect.position, triIntersect.distance);
			}
		} else {
			if (node.Right() != -1) { stack.push_back({ node.Right(),nodeIndex.second + 1 }); }
			if (node.Left() != -1) { stack.push_back({ node.Left(),nodeIndex.second + 1 }); }
		}
	}

	return minDist;
	
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
	stack.push_back(m_nodes.size() - 1); // ルートノードから開始
	Vector<BVH::IntersectResult> hit;
	while (!stack.empty()) {
		int nodeIndex = stack.back();
		stack.pop_back();

		const Node& node = m_nodes[nodeIndex];

		auto intersect = ray.Intersect(BDB(node.MinBox(), node.MaxBox()));
		// AABBと交差しない場合はスキップ
		if (!intersect.success) { continue; }

		if (node.Left() == -1 && node.Right() == -1) {
			// リーフノードなら交差リストに追加
			auto face = GetFace(m_pHalfEdge, node.Triangle());
			auto triIntersect = ray.Intersect(face.pos0, face.pos1, face.pos2, false);
			if (triIntersect.success) {
				hit.push_back(BVH::IntersectResult(node.Triangle(), triIntersect.position, triIntersect.distance));
			}
		} else {
			// 内部ノードなら子ノードをスタックに追加（右を先に入れると左が先に処理される）
			if (node.Right() != -1) stack.push_back(node.Right());
			if (node.Left() != -1) stack.push_back(node.Left());
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

	// リーフノードなら出力
	if (node.Left() == -1 && node.Right() == -1) {
		leafNum++;
		return;
	}

	// 左の子をたどる
	if (node.Left() != -1) {
		CountLeafNodes(node.Left(), leafNum);
	}

	// 右の子をたどる
	if (node.Right() != -1) {
		CountLeafNodes(node.Right(), leafNum);
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
void BVH::CreateGPUBuffer()
{
	if (m_gpu.pBuffer) { return; }
	m_gpu.pBuffer = std::make_unique<GLBuffer>();
	//m_gpu.pBuffer->Create()
}
void BVH::ShowUI(RenderNode* pNode, UIContext& ui)
{
	if (ImGui::SliderInt("ShowLevel", &m_ui.showLevel, -1, m_levelRange.size() - 1)) {
		if (m_ui.showLevel == -1) { return; }
		size_t offset = 0;
		Vector<Vector3> position;
		Vector<unsigned int> indexs;
		for (int i = m_levelRange[m_ui.showLevel].first; i < m_levelRange[m_ui.showLevel].second; i++) {
			auto cube = Cube::CreateLine(m_nodes[i].MinBox(), m_nodes[i].MaxBox());
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
		auto pPrimitive = std::make_shared<PrimitiveNode>("BVH", pCube, ColorUtility::CreatePrimary(2));
		pPrimitive->SetMatrix(m_pHalfEdge->GetMatrix());
		m_pHalfEdge->AddNode(pPrimitive);
	}
}
}
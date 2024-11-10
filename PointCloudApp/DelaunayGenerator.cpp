#include "DelaunayGenerator.h"
#include "PrimitiveNode.h"
#include "PointCloudNode.h"
#include "PointCloud.h"
#include "Primitives.h"
#include "Utility.h"
namespace KI
{


DelaunayGenerator::DelaunayGenerator(PointCloudNode* pPointCloud)
	:m_pPointCloud(pPointCloud)
	,m_positions(m_pPointCloud->GetData()->Position())
{
}

DelaunayGenerator::~DelaunayGenerator()
{

}

Vector<unsigned int> DelaunayGenerator::Execute2D(int iterate)
{
	m_HugeTriangle = CreateHugeTriangle2D();

	m_Delaunay.clear();
	m_Delaunay.push_back(IndexedTriangle(0, HUGE_POS0, HUGE_POS1));
	m_Delaunay.push_back(IndexedTriangle(0, HUGE_POS1, HUGE_POS2));
	m_Delaunay.push_back(IndexedTriangle(0, HUGE_POS2, HUGE_POS0));
	if (m_positions.size() < iterate) { iterate = m_positions.size(); }
	for (int i = 1; i < iterate; i++) {
		Division(i);
	}

	RemoveHugeTriangle();
	return Vector<unsigned int>();
}


bool DelaunayGenerator::InnerByCircle(const DelaunayGenerator::Circumscribe& circle, const Vector3& point)
{
	return glm::length2(point - circle.center) < circle.radius * circle.radius;
}


void DelaunayGenerator::Division(int index)
{
	const auto& addPoint = m_positions[index];
	Vector<IndexedTriangle> addTri;
	for (auto it = m_Delaunay.begin(); it != m_Delaunay.end();) {
		if (InnerByCircle(CalcCircumscribedCircle(*it), addPoint)) {
			addTri.push_back(IndexedTriangle(index, it->pos0, it->pos1));
			addTri.push_back(IndexedTriangle(index, it->pos1, it->pos2));
			addTri.push_back(IndexedTriangle(index, it->pos2, it->pos0));
			it = m_Delaunay.erase(it);
		} else {
			++it;
		}
	}

	for (int i = 0; i < addTri.size(); i++) {
		bool isUnique = true;
		for (int j = 0; j < addTri.size(); j++) {
			if (i == j) { continue; }
			if (addTri[i].IsSame(addTri[j])) {
				isUnique = false;
				break;
			}
		}

		if (isUnique) {
			m_Delaunay.push_back(addTri[i]);
		}
	}
}
void DelaunayGenerator::RemoveHugeTriangle()
{
	for (auto it = m_Delaunay.begin(); it != m_Delaunay.end();) {
		if (it->HasHuge()) {
			it = m_Delaunay.erase(it);
		} else {
			++it;
		}
	}
}
DelaunayGenerator::Triangle DelaunayGenerator::CreateHugeTriangle2D()
{
	Vector3 center = Vector3(0);
	for (const auto& p : m_positions) {
		center += p;
	}

	center /= m_positions.size();

	float maxLength = 0;
	Vector3 maxPos;
	for (const auto& p : m_positions) {
		auto len = glm::length2(p - center);
		if (maxLength < len) {
			maxLength = len;
			maxPos = p;
		}
	}

	// 点を覆う三角形を構築
	Triangle tri;
	auto radDir = (center - maxPos) * 1.5f;
	maxPos *= 1.5f;
	float radius = glm::length(radDir);
	float root3 = sqrtf(radius);
	tri.pos0 = center + (root3 * radDir);
	tri.pos1 = maxPos + (root3 * Vector3(radDir.x, -radDir.y, 0.0));
	tri.pos2 = maxPos - (root3 * Vector3(radDir.x, -radDir.y, 0.0));

	return tri;
}

// 3点の座標を入力として、外接円の中心と半径を計算する関数
DelaunayGenerator::Circumscribe DelaunayGenerator::CalcCircumscribedCircle(const IndexedTriangle& triangle)
{
	return CalcCircumscribedCircle(triangle.Convert(this));
}
DelaunayGenerator::Circumscribe DelaunayGenerator::CalcCircumscribedCircle(const Triangle& triangle)
{
	const auto& pos1 = triangle.pos0;
	const auto& pos2 = triangle.pos1;
	const auto& pos3 = triangle.pos2;


	// 三角形の各辺の長さを計算
	float dA = glm::length2(pos1);
	float dB = glm::length2(pos2);
	float dC = glm::length2(pos3);

	// 行列式を使って外心の座標を求める
	float aux1 = (dA * (pos3.y - pos2.y) + dB * (pos1.y - pos3.y) + dC * (pos2.y - pos1.y));
	float aux2 = -(dA * (pos3.x - pos2.x) + dB * (pos1.x - pos3.x) + dC * (pos2.x - pos1.x));
	float div = (2 * (pos1.x * (pos3.y - pos2.y) + pos2.x * (pos1.y - pos3.y) + pos3.x * (pos2.y - pos1.y)));

	Circumscribe c;
	// 外心のx座標とy座標
	c.center = Vector3(aux1 / div, aux2 / div, 0.0f);
	// 半径を計算（外心から任意の頂点までの距離）
	c.radius = glm::length(c.center - pos1);
	return  c;

}

int g_iterateNum = 0;
void DelaunayGenerator::ShowUI()
{
	if (ImGui::SliderInt("DelaunayGenerator", &g_iterateNum, 3, m_positions.size())) {
		Execute2D(g_iterateNum);
		// Huge Triangle
		//{
		//	auto pTriangle = std::make_shared<Primitive>();
		//	Vector<Vector3> pos;
		//	pos.push_back(m_HugeTriangle.pos0);
		//	pos.push_back(m_HugeTriangle.pos1);
		//	pos.push_back(m_HugeTriangle.pos2);
		//	Vector<unsigned int> index;
		//	index.push_back(0); index.push_back(1);
		//	index.push_back(1); index.push_back(2);
		//	index.push_back(2); index.push_back(0);

		//	pTriangle->SetPosition(std::move(pos));
		//	pTriangle->SetIndex(std::move(index));

		//	pTriangle->SetType(GL_LINES);
		//	m_pPointCloud->AddNode(std::make_shared<PrimitiveNode>("DelaunayTriangle", pTriangle, ColorUtility::CreatePrimary(4)));

		//	auto c = CalcCircumscribedCircle(m_HugeTriangle);
		//	Shared<Primitive> pCircle = std::make_shared<Circle>(c.radius, c.center);
		//	m_pPointCloud->AddNode(std::make_shared<PrimitiveNode>("DelaunayCircle", pCircle, ColorUtility::CreatePrimary(3)));
		//}

		// Delaunay Circle;
		{
			//Shared<Primitive> pCircle = std::make_shared<Circle>(1, Vector3(0.0f));
			//auto pNode = std::make_shared<InstancedPrimitiveNode>("InstancedNode", pCircle, ColorUtility::CreatePrimary(6));
			//Vector<Matrix4x4> matrixs;
			//for (const auto& delaunay : m_Delaunay) {
			//	auto c = CalcCircumscribedCircle(delaunay);
			//	matrixs.push_back(glmUtil::CreateTransform(c.radius, c.center));
			//}

			//pNode->SetMatrixs(std::move(matrixs));
			//m_pPointCloud->AddNode(pNode);
		
			Shared<Primitive> pTriangle = std::make_shared<Primitive>();
			Vector<Vector3> pos(m_Delaunay.size() * 3 * 2);
			int counter = 0;
			int num = 0;
			for (const auto& tri : m_Delaunay) {
				const auto& triPos = tri.Convert(this);
				pos[counter++] = triPos.pos0;
				pos[counter++] = triPos.pos1;

				pos[counter++] = triPos.pos1;
				pos[counter++] = triPos.pos2;

				pos[counter++] = triPos.pos2;
				pos[counter++] = triPos.pos0;
			}
			pos.resize(counter);

			pTriangle->SetPosition(std::move(pos));
			pTriangle->SetType(GL_LINES);

			m_pPointCloud->AddNode(std::make_shared<PrimitiveNode>("DelaunayTriangle", pTriangle, ColorUtility::CreatePrimary(2)));
		}

	}
}
}
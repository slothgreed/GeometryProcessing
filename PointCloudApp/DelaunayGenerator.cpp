#include "DelaunayGenerator.h"
#include "PrimitiveNode.h"
#include "PointCloudNode.h"
#include "PointCloud.h"
#include "Primitives.h"
#include "Utility.h"
#include "KIMath.h"




#define CGAL_NO_GMP 0
#define CGAL_NO_MPFR 0
#define CGAL_DISABLE_GMP 1
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>

#include <vector>
#include <list>
#include <iostream>
namespace KI
{
// ----------------------
// Domain marking helper
// ----------------------
struct FaceInfo2
{
	int nesting_level = -1;      // -1 = unvisited
	bool in_domain() const { return nesting_level % 2 == 1; }
};

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point = K::Point_2;

using Vb = CGAL::Triangulation_vertex_base_2<K>;
using Fbb = CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K>;
using Fb = CGAL::Constrained_triangulation_face_base_2<K, Fbb>;
using TDS = CGAL::Triangulation_data_structure_2<Vb, Fb>;
using Itag = CGAL::Exact_intersections_tag;
using CDT = CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>;

static void
mark_component(CDT& cdt, CDT::Face_handle start, int level, std::list<CDT::Edge>& border)
{
	std::list<CDT::Face_handle> queue;
	queue.push_back(start);

	while (!queue.empty()) {
		CDT::Face_handle fh = queue.front();
		queue.pop_front();

		if (fh->info().nesting_level != -1) continue;
		fh->info().nesting_level = level;

		for (int i = 0; i < 3; ++i) {
			CDT::Edge e(fh, i);
			CDT::Face_handle n = fh->neighbor(i);

			if (n->info().nesting_level == -1) {
				if (cdt.is_constrained(e))
					border.push_back(e);
				else
					queue.push_back(n);
			}
		}
	}
}

static void
mark_domains(CDT& cdt)
{
	// init
	for (auto f = cdt.all_faces_begin(); f != cdt.all_faces_end(); ++f)
		f->info().nesting_level = -1;

	std::list<CDT::Edge> border;
	// outside is level 0
	mark_component(cdt, cdt.infinite_face(), 0, border);

	while (!border.empty()) {
		CDT::Edge e = border.front();
		border.pop_front();

		CDT::Face_handle n = e.first->neighbor(e.second);
		if (n->info().nesting_level == -1) {
			// Cross a constrained edge => toggle level
			mark_component(cdt, n, e.first->info().nesting_level + 1, border);
		}
	}
}

// ----------------------
// Insert polygon constraint (closed polyline)
// ----------------------
static void insert_closed_polyline(CDT& cdt, const Vector<Vector3>& poly)
{
	const int n = (int)poly.size();
	if (n < 3) return;
	for (int i = 0; i < n; i+=2) {
		auto a = poly[i];
		auto b = poly[i + 1];
		cdt.insert_constraint(
			Point((double)a.x, (double)a.y),
			Point((double)b.x, (double)b.y));
	}
	return;
	

	for (int i = 0; i < n; ++i) {
		auto a = poly[i];
		auto b = poly[(i + 1) % n];
		cdt.insert_constraint(
			Point((double)a.x, (double)a.y),
			Point((double)b.x, (double)b.y));
	}
}
Vector<Vector3> DelaunayGenerator::Execute2D_CGAL(const Vector<Vector3>& polyline, const Vector<const Vector<Vector3>*>& inPolyline, int iterate)
{
	CDT cdt;
	// 1) Insert constraints
	insert_closed_polyline(cdt, polyline);
	for (const auto& inner : inPolyline) {
		insert_closed_polyline(cdt, *inner);
	}

	// 2) Refine (optional): you can add extra points if you want denser triangulation
	// cdt.insert(Point( ... ));

	// 3) Mark domain (inside outer minus holes)
	mark_domains(cdt);

	Vector<Vector3> triangle;
	// 4) Extract triangles in domain
	for (auto fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit) {
		if (!fit->info().in_domain()) continue;

		Point p0 = fit->vertex(0)->point();
		Point p1 = fit->vertex(1)->point();
		Point p2 = fit->vertex(2)->point();

		triangle.push_back(Vector3(p0.x(), p0.y(), 0.0f));
		triangle.push_back(Vector3(p1.x(), p1.y(), 0.0f));
		triangle.push_back(Vector3(p2.x(), p2.y(), 0.0f));
	}

	return triangle;
}
int main2()
{
	//CDT cdt;

	//// ---- Example input ----
	//// Outer boundary (concave)
	//std::vector<Point> outer = {
	//  {0,0}, {10,0}, {10,10}, {6,10}, {6,4}, {4,4}, {4,10}, {0,10}
	//};

	//// Hole boundary (a rectangle)
	//std::vector<Point> hole1 = {
	//  {2,2}, {3,2}, {3,3}, {2,3}
	//};

	//// Another hole (triangle)
	//std::vector<Point> hole2 = {
	//  {7,2}, {9,2}, {8,3.5}
	//};

	//// 1) Insert constraints
	//insert_closed_polyline(cdt, outer);
	//insert_closed_polyline(cdt, hole1);
	//insert_closed_polyline(cdt, hole2);

	//// 2) Refine (optional): you can add extra points if you want denser triangulation
	//// cdt.insert(Point( ... ));

	//// 3) Mark domain (inside outer minus holes)
	//mark_domains(cdt);

	//// 4) Extract triangles in domain
	//int triCount = 0;
	//for (auto fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit) {
	//	if (!fit->info().in_domain()) continue;

	//	Point p0 = fit->vertex(0)->point();
	//	Point p1 = fit->vertex(1)->point();
	//	Point p2 = fit->vertex(2)->point();

	//	// do something (here: print)
	//	std::cout << "T " << p0 << " " << p1 << " " << p2 << "\n";
	//	++triCount;
	//}

	//std::cerr << "triangles in domain = " << triCount << "\n";
	//return 0;
	return 0;
}
Vector<Vector3> DelaunayGenerator::Execute2D_CGAL()
{
	if (m_target->size() < 3) { return Vector<Vector3>(); }
	return Execute2D_CGAL(*m_target, m_inner, -1);
}

Vector<unsigned int> DelaunayGenerator::Execute2D()
{
	if (m_target == nullptr) { return Vector<unsigned int>(); }
	if (m_target->size() < 3) { return Vector<unsigned int>(); }
	if (MathHelper::IsZ(MathHelper::CalcNormal(m_target->at(0), m_target->at(1), m_target->at(2)))) {
		return Execute2D(*m_target, m_inner, -1);
	} else {
		auto normal = MathHelper::CalcNormal(m_target->at(0), m_target->at(1), m_target->at(2));
		auto matrix = MathHelper::CreateZAxisMatrix(normal);
		auto target2D = MathHelper::Rotate(*m_target, matrix);
		Vector<Vector<Vector3>> inner2Ds;
		for (int i = 0; i < m_inner.size(); i++) {
			inner2Ds.push_back(MathHelper::Rotate(*m_inner[i], matrix));
		}

		DelaunayGenerator delaunay2D;
		delaunay2D.SetTarget(&target2D);
		for (int i = 0; i < m_inner.size(); i++) {
			delaunay2D.AddInner(&inner2Ds[i]);
		}
		delaunay2D.Execute2D();
		m_Delaunay = delaunay2D.m_Delaunay;
		return Vector<unsigned int>();
	}
}
Vector<unsigned int> DelaunayGenerator::GetResult() const
{
	Vector<unsigned int> indexs;
	auto targetNormal = MathHelper::CalcNormal(m_target->at(0), m_target->at(1), m_target->at(2));
	for (const auto& delaunay : m_Delaunay) {
		auto tri = delaunay.Convert(this);
		auto inner = glm::dot(targetNormal, MathHelper::CalcNormal(tri.pos0, tri.pos1, tri.pos2));
		if (MathHelper::IsOne(inner)) {
			indexs.push_back(delaunay.pos0);
			indexs.push_back(delaunay.pos1);
			indexs.push_back(delaunay.pos2);
		} else {
			indexs.push_back(delaunay.pos0);
			indexs.push_back(delaunay.pos2);
			indexs.push_back(delaunay.pos1);
		}
	}
	return indexs;
}
Vector<Vector3> DelaunayGenerator::Execute2DTriangles()
{
	if (m_target == nullptr) { return Vector<Vector3>(); }
	Execute2D();
	Vector<Vector3> triangles;
	auto targetNormal = MathHelper::CalcNormal(m_target->at(0), m_target->at(1), m_target->at(2));
	for (const auto& delaunay : m_Delaunay) {
		auto tri = delaunay.Convert(this);
		auto inner = glm::dot(targetNormal, MathHelper::CalcNormal(tri.pos0, tri.pos1, tri.pos2));
		if (MathHelper::IsOne(inner)) {
			triangles.push_back(tri.pos0);
			triangles.push_back(tri.pos1);
			triangles.push_back(tri.pos2);
		} else {
			triangles.push_back(tri.pos0);
			triangles.push_back(tri.pos2);
			triangles.push_back(tri.pos1);
		}
	}

	return triangles;

}

void DelaunayGenerator::OutputTargetInner()
{
	DebugPrintf::Vec3Array("Target", *m_target);
	for (int i = 0; i < m_inner.size(); i++) {
		DebugPrintf::Vec3Array("Inner", *m_inner[i]);
	}
}
Vector<unsigned int> DelaunayGenerator::Execute2D(const Vector<Vector3>& position, int iterate)
{
	m_HugeTriangle = CreateHugeTriangle(position);

	m_Delaunay.clear();
	m_Delaunay.push_back(IndexedTriangle(0, HUGE_POS0, HUGE_POS1));
	m_Delaunay.push_back(IndexedTriangle(0, HUGE_POS1, HUGE_POS2));
	m_Delaunay.push_back(IndexedTriangle(0, HUGE_POS2, HUGE_POS0));
	if (position.size() < iterate) { iterate = position.size(); }
	for (int i = 1; i < iterate; i++) {
		Division(position, i);
	}

	RemoveHugeTriangle();
	// position外にある三角形を削除する。
	for (auto it = m_Delaunay.begin(); it != m_Delaunay.end();) {
		if (!MathHelper::InPolyline(position, it->GetGravity(this), true)) {
			it = m_Delaunay.erase(it);
		} else {
			++it;
		}
	}

	return GetResult();
}

DelaunayGenerator::IndexedTriangle DelaunayGenerator::IndexedTriangle::Create(int p0, int p1, int p2, const Vector<IndexedEdge>& constraints)
{
	IndexedTriangle tri;
	tri.pos0 = p0;
	tri.pos1 = p1;
	tri.pos2 = p2;

	auto e0 = IndexedEdge(p0, p1);
	auto e1 = IndexedEdge(p1, p2);
	auto e2 = IndexedEdge(p2, p0);
	tri.const0 = false;
	tri.const1 = false;
	tri.const2 = false;

	for (size_t i = 0; i < constraints.size(); i++) {
		if (constraints[i] == e0) { tri.const0 = true; }
		if (constraints[i] == e1) { tri.const1 = true; }
		if (constraints[i] == e2) { tri.const2 = true; }
	}
	return tri;
}

Vector<unsigned int> DelaunayGenerator::Execute2D(const Vector<Vector3>& polyline, const Vector<const Vector<Vector3>*>& inPolyline, int iterate)
{
	Vector<Vector3> merge = polyline;
	
	for (size_t i = 0; i < inPolyline.size(); i++) {
		for (size_t j = 0; j < inPolyline[i]->size(); j++) {
			merge.push_back(inPolyline[i]->at(j));
		}
	}

	if (iterate == -1) { iterate = merge.size(); }
	m_HugeTriangle = CreateHugeTriangle(merge);
	m_Delaunay.clear();
	m_Delaunay.push_back(IndexedTriangle(0, HUGE_POS0, HUGE_POS1));
	m_Delaunay.push_back(IndexedTriangle(0, HUGE_POS1, HUGE_POS2));
	m_Delaunay.push_back(IndexedTriangle(0, HUGE_POS2, HUGE_POS0));
	if (merge.size() < iterate) { iterate = merge.size(); }
	for (int i = 1; i < merge.size(); i++) {
		Division(merge, i);
	}


	int edgeCount = 0;
	for (int i = 0; i < polyline.size() - 1; i++) {
		m_ConstraintEdge.insert(IndexedEdge(edgeCount, edgeCount + 1));
		edgeCount++;
	}

	edgeCount++;
	for (int i = 0; i < inPolyline.size(); i++) {
		for (int j = 0; j < inPolyline[i]->size(); j++) {
			m_ConstraintEdge.insert(IndexedEdge(edgeCount, edgeCount + 1));
			edgeCount++;
		}

		edgeCount++;
	}

	for (auto& delaunay : m_Delaunay) {
		if (m_ConstraintEdge.erase(delaunay.GetEdge0()) > 0) { delaunay.const0 = true; }
		if (m_ConstraintEdge.erase(delaunay.GetEdge1()) > 0) { delaunay.const1 = true; }
		if (m_ConstraintEdge.erase(delaunay.GetEdge2()) > 0) { delaunay.const2 = true; }
	}

	for (const auto& edge : m_ConstraintEdge) {
		
	}


	RemoveHugeTriangle();


	// polyline外にある三角形を削除する。
	//for (auto it = m_Delaunay.begin(); it != m_Delaunay.end();) {
	//	if (!MathHelper::InPolyline(polyline, it->GetGravity(this), true)) {
	//		it = m_Delaunay.erase(it);
	//	} else {
	//		++it;
	//	}
	//}

	// inner内にある三角形を削除する。
	for (int i = 0; i < inPolyline.size(); i++) {
		for (auto it = m_Delaunay.begin(); it != m_Delaunay.end();) {
			if (MathHelper::InPolyline(*inPolyline[i], it->GetGravity(this), true)) {
				it = m_Delaunay.erase(it);
			} else {
				++it;
			}
		}
	}

	return GetResult();
}

bool DelaunayGenerator::InnerByCircle(const DelaunayGenerator::Circumscribe& circle, const Vector3& point)
{
	float dist2 = glm::length2(point - circle.center);
	float radius2 = circle.radius * circle.radius;

	const float EPS = 1e-6; // 浮動小数点誤差対策

	if (dist2 < radius2 - EPS) return true;   // 明確に内側
	if (dist2 > radius2 + EPS) return false;  // 明確に外側

	// ---- タイブレーク規則 ----
	// 外接円上にちょうどある場合
	// 例: 座標の辞書順で内側扱いにする
	if (point.x < circle.center.x ||
	   (point.x == circle.center.x && point.y < circle.center.y)) {
		return true;   // 内側に含める
	}

	return false; // 外側にする
	return glm::length2(point - circle.center) < circle.radius * circle.radius;
}

void DelaunayGenerator::DivisionConstraint(const Vector<Vector3>& position, int index)
{
	std::unordered_set<IndexedEdge, IndexedEdge::Hash> boundEdge; // 境界辺
	for (auto it = m_Delaunay.begin(); it != m_Delaunay.end();) {
		auto inner = InnerByCircle(CalcCircumscribedCircle(*it), position[index]);
		if (inner) {
			auto e0 = IndexedEdge(it->pos0, it->pos1);
			auto e1 = IndexedEdge(it->pos1, it->pos2);
			auto e2 = IndexedEdge(it->pos2, it->pos0);
			if (!boundEdge.insert(e0).second) { boundEdge.erase(e0); }
			if (!boundEdge.insert(e1).second) { boundEdge.erase(e1); }
			if (!boundEdge.insert(e2).second) { boundEdge.erase(e2); }
			it = m_Delaunay.erase(it);
		} else {
			++it;
		}
	}

	for (auto& edge : boundEdge) {
		m_Delaunay.push_back(IndexedTriangle(index, edge.pos0, edge.pos1));
	}
}
void DelaunayGenerator::Division(const Vector<Vector3>& position, int index)
{
	int pattern = 0;
	if (pattern == 0) {
		std::unordered_set<IndexedEdge, IndexedEdge::Hash> boundEdge; // 境界辺
		for (auto it = m_Delaunay.begin(); it != m_Delaunay.end();) {
			auto inner = InnerByCircle(CalcCircumscribedCircle(*it), position[index]);
			if (inner) {
				auto e0 = IndexedEdge(it->pos0, it->pos1);
				auto e1 = IndexedEdge(it->pos1, it->pos2);
				auto e2 = IndexedEdge(it->pos2, it->pos0);
				if (!boundEdge.insert(e0).second) { boundEdge.erase(e0); }
				if (!boundEdge.insert(e1).second) { boundEdge.erase(e1); }
				if (!boundEdge.insert(e2).second) { boundEdge.erase(e2); }
				it = m_Delaunay.erase(it);
			} else {
				++it;
			}
		}

		for (auto& edge : boundEdge) {
			m_Delaunay.push_back(IndexedTriangle(index, edge.pos0, edge.pos1));
		}
	} else if (pattern == 1) {

		Vector<IndexedTriangle> addTri;
		for (auto it = m_Delaunay.begin(); it != m_Delaunay.end();) {
			if (InnerByCircle(CalcCircumscribedCircle(*it), position[index])) {
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
DelaunayGenerator::Triangle DelaunayGenerator::CreateHugeTriangle(const Vector<Vector3>& position)
{
	BDB bdb;
	for (const auto& p : position) {
		bdb.Add(p);
	}

	auto center = bdb.Center();
	auto bdbLength = bdb.MaxLength() * 10.0f;
	float maxLength = 0;
	Triangle tri;
	tri.pos0 = center + Vector3(-bdbLength, -bdbLength, 0);
	tri.pos1 = center + Vector3(bdbLength, -bdbLength, 0);
	tri.pos2 = center + Vector3(0, bdbLength, 0);
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

Vector3 DelaunayGenerator::IndexedTriangle::Convert(int index, const DelaunayGenerator* pGen) const
{
	if (index == HUGE_POS0) { return pGen->m_HugeTriangle.pos0; }
	if (index == HUGE_POS1) { return pGen->m_HugeTriangle.pos1; }
	if (index == HUGE_POS2) { return pGen->m_HugeTriangle.pos2; }

	if (index < pGen->m_target->size()) {
		return pGen->m_target->at(index);
	}
	int sum = pGen->m_target->size();
	for (int i = 0; i < pGen->m_inner.size(); i++) {
		if (index < sum + pGen->m_inner[i]->size()) {
			return pGen->m_inner[i]->at(index - sum);
		}
		sum += pGen->m_inner[i]->size();
	}

	assert(0);
	return Vector3();
}

DelaunayGenerator::Triangle DelaunayGenerator::IndexedTriangle::Convert(const DelaunayGenerator* pGen) const
{
	Triangle tri;
	tri.pos0 = Convert(pos0, pGen);
	tri.pos1 = Convert(pos1, pGen);
	tri.pos2 = Convert(pos2, pGen);

	return tri;
}

Vector3 DelaunayGenerator::IndexedTriangle::GetGravity(const DelaunayGenerator* pGen) const
{
	Triangle tri = Convert(pGen);
	return (tri.pos0 + tri.pos1 + tri.pos2) / 3.0f;
}

void DelaunayGenerator::ShowUI(RenderNode* pNode, UIContext& ui)
{
	if (ImGui::Button("Delete")) {
		pNode->RemoveNode("HugeTriangle");
		pNode->RemoveNode("DelaunayCircle");
		pNode->RemoveNode("InstancedNode");
		pNode->RemoveNode("DelaunayTriangle");

	}
	if (ImGui::Button("DelaunayGenerator_CGAL")) {
		auto triangles = Execute2D_CGAL(*m_target, m_inner, 0);
		Shared<Primitive> pTriangle = std::make_shared<Primitive>();
		pTriangle->SetPosition(std::move(triangles));
		pTriangle->SetType(GL_TRIANGLES);

		pNode->AddNode(std::make_shared<PrimitiveNode>("DelaunayTriangle", pTriangle, ColorUtility::CreatePrimary(2)));
	}
	if (ImGui::SliderInt("DelaunayGenerator", &m_ui.iterate, 3, m_target->size())) {
		Execute2D(*m_target, m_inner, m_ui.iterate);
		// Huge Triangle
		{
			auto pTriangle = std::make_shared<Primitive>();
			Vector<Vector3> pos;
			pos.push_back(m_HugeTriangle.pos0);
			pos.push_back(m_HugeTriangle.pos1);
			pos.push_back(m_HugeTriangle.pos2);
			Vector<unsigned int> index;
			index.push_back(0); index.push_back(1);
			index.push_back(1); index.push_back(2);
			index.push_back(2); index.push_back(0);

			pTriangle->SetPosition(std::move(pos));
			pTriangle->SetIndex(std::move(index));

			pTriangle->SetType(GL_LINES);
			pNode->AddNode(std::make_shared<PrimitiveNode>("HugeTriangle", pTriangle, ColorUtility::CreatePrimary(4)));

			auto c = CalcCircumscribedCircle(m_HugeTriangle);
			Shared<Primitive> pCircle = std::make_shared<Circle>(c.radius, c.center);
			pNode->AddNode(std::make_shared<PrimitiveNode>("DelaunayCircle", pCircle, ColorUtility::CreatePrimary(3)));
		}
		// Delaunay Circle;
		{
			//Shared<Primitive> pCircle = std::make_shared<Circle>(1, Vector3(0.0f));
			//auto pInstanceNode = std::make_shared<InstancedPrimitiveNode>("InstancedNode", pCircle, ColorUtility::CreatePrimary(6));
			//Vector<Matrix4x4> matrixs;
			//for (const auto& delaunay : m_Delaunay) {
			//	auto c = CalcCircumscribedCircle(positions, delaunay);
			//	matrixs.push_back(glmUtil::CreateTransform(c.radius, c.center));
			//}

			//pInstanceNode->SetMatrixs(std::move(matrixs));
			//pNode->AddNode(pInstanceNode);
		
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

			pNode->AddNode(std::make_shared<PrimitiveNode>("DelaunayTriangle", pTriangle, ColorUtility::CreatePrimary(2)));
		}
	}
}

Delaunay3DGenerator::Circumsphere Delaunay3DGenerator::CreateCircumsphere(const Tetrahedron& tet)
{
	auto det3x3 = [](const Vector3& u, const Vector3& v, const Vector3& w)
	{
		return u.x * (v.y * w.z - v.z * w.y)
			- u.y * (v.x * w.z - v.z * w.x)
			+ u.z * (v.x * w.y - v.y * w.x);
	};

	auto ba = tet.p1 - tet.p0;
	auto ca = tet.p2 - tet.p0;
	auto da = tet.p3 - tet.p0;

	auto ba2 = ba * 0.5f;
	auto ca2 = ca * 0.5f;
	auto da2 = da * 0.5f;

	float det = det3x3(ba, ca, da);
	if (std::abs(det) < 1e-10) {
		return Circumsphere(); // 退化
	}

	auto cross_cd = Vector3(
		ca.y * da.z - ca.z * da.y,
		ca.z * da.x - ca.x * da.z,
		ca.x * da.y - ca.y * da.x);
	auto numerator = Vector3(
		  glm::length2(ba) * (ca.y * da.z - ca.z * da.y)
		- glm::length2(ca) * (ba.y * da.z - ba.z * da.y)
		+ glm::length2(da) * (ba.y * ca.z - ba.z * ca.y),
		- glm::length2(ba) * (ca.x * da.z - ca.z * da.x)
		+ glm::length2(ca) * (ba.x * da.z - ba.z * da.x)
		- glm::length2(da) * (ba.x * ca.z - ba.z * ca.x),
		  glm::length2(ba) * (ca.x * da.y - ca.y * da.x)
		- glm::length2(ca) * (ba.x * da.y - ba.y * da.x)
		+ glm::length2(da) * (ba.x * ca.y - ba.y * ca.x));

	auto center = Vector3(tet.p0 + 0.5f * numerator / det);
	float radius = glm::length(center - tet.p0);

	return Circumsphere(center, radius);
}
Delaunay3DGenerator::Tetrahedron Delaunay3DGenerator::CreateHugeTetrahedron(const BDB& bdb) const
{
	auto length = bdb.MaxLength();
	Tetrahedron t;
	t.p0 = Vector3(bdb.Center().x - length, bdb.Center().y - length, bdb.Center().z - length);
	t.p1 = Vector3(bdb.Center().x + length, bdb.Center().y + length, bdb.Center().z - length);
	t.p2 = Vector3(bdb.Center().x + length, bdb.Center().y - length, bdb.Center().z + length);
	t.p3 = Vector3(bdb.Center().x - length, bdb.Center().y + length, bdb.Center().z + length);

	return t;
}
}
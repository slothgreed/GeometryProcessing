#ifndef DELAUNAY_GENERATOR
#define DELAUNAY_GENERATOR
#include "IAlgorithm.h"
namespace KI
{

class PointCloudNode;
class DebugNode;
class DelaunayGenerator : public IAlgorithm
{
public:
	DelaunayGenerator()
		: m_target(nullptr)
		, m_ccw(true)
	{
	}
	~DelaunayGenerator() {};

	ALGORITHM_TYPE GetType() { return ALGORITHM_DELAUNAY; }
	struct UI
	{
		UI()
			: iterate(0)
			, notify(false)
		{

		}
		bool notify;
		int iterate;
	};


	Vector<unsigned int> Execute2D();
	void ShowUI(RenderNode* pNode, UIContext& ui);
	void Clear() { m_target = nullptr; m_inner.clear(); }
	void SetTarget(const Vector<Vector3>* position, bool ccw) { m_target = position; m_ccw = ccw; }
	void SetTarget(const Vector<Vector3>* position) { m_target = position; }
	void AddInner(const Vector<Vector3>* inner) { m_inner.push_back(inner); }
private:
	Vector<unsigned int> Execute2D(const Vector<Vector3>& position, int iterate);
	Vector<unsigned int> Execute2D(const Vector<Vector3>& polyline, const Vector<const Vector<Vector3>*>& inPolyline, int iterate);


	struct Circumscribe
	{
		Circumscribe() :radius(0) {}
		float radius;
		Vector3 center;
	};

	struct Edge
	{
		Edge() {}
		Edge(const Vector3& p0, const Vector3& p1) : pos0(p0), pos1(p1) {}

		Vector3 pos0;
		Vector3 pos1;
	};

	struct Triangle
	{
		Triangle() {}
		Triangle(const Vector3& p0, const Vector3& p1, const Vector3& p2) : pos0(p0), pos1(p1), pos2(p2) {}
		
		Vector3 pos0;
		Vector3 pos1;
		Vector3 pos2;
	};

	static const int HUGE_POS0 = -1;
	static const int HUGE_POS1 = -2;
	static const int HUGE_POS2 = -3;

	struct IndexedEdge
	{
		friend class DelaunayGenerator;
		IndexedEdge() : pos0(-1), pos1(-1) {}
		IndexedEdge(int p0, int p1) : pos0(p0), pos1(p1){}
		int pos0;
		int pos1;


		Edge Convert(const Vector<Vector3>& positions, DelaunayGenerator* pGen) const
		{
			Edge edge;
			if (pos0 == HUGE_POS0) { edge.pos0 = pGen->m_HugeTriangle.pos0; }
			else if (pos0 == HUGE_POS1) { edge.pos0 = pGen->m_HugeTriangle.pos1; }
			else if (pos0 == HUGE_POS2) { edge.pos0 = pGen->m_HugeTriangle.pos2; }
			else { edge.pos0 = positions[pos0]; }

			if (pos1 == HUGE_POS0) { edge.pos1 = pGen->m_HugeTriangle.pos0; }
			else if (pos1 == HUGE_POS1) { edge.pos1 = pGen->m_HugeTriangle.pos1; }
			else if (pos1 == HUGE_POS2) { edge.pos1 = pGen->m_HugeTriangle.pos2; }
			else { edge.pos1 = positions[pos1]; }

			return edge;
		}

	
		bool HasHuge() const
		{
			return
				pos0 == HUGE_POS0 || pos0 == HUGE_POS1 || pos0 == HUGE_POS2 ||
				pos1 == HUGE_POS0 || pos1 == HUGE_POS1 || pos1 == HUGE_POS2;
		}
		bool IsSame(const IndexedEdge& tri) const
		{
			if (pos0 == tri.pos0 && pos1 == tri.pos1) { return true; }
			if (pos1 == tri.pos0 && pos0 == tri.pos1) { return true; }

			return false;
		}


		size_t GetHash() const
		{
			int p0 = std::min(pos0, pos1);
			int p1 = std::max(pos0, pos1);
			return
				std::hash<int>()(p0) ^
				std::hash<int>()(p1) << 1;
		}

		bool operator==(const IndexedEdge& e) const
		{
			return IsSame(e);
		}

		struct Hash
		{
			std::size_t operator()(const IndexedEdge& e) const 
			{
				return e.GetHash();
			}
		};
	};


	struct IndexedTriangle
	{
		friend class DelaunayGenerator;
		IndexedTriangle() : pos0(-1), pos1(-1), pos2(-1) {}
		IndexedTriangle(int p0, int p1, int p2) : pos0(p0), pos1(p1), pos2(p2), const0(false), const1(false), const2(false) {}
		int pos0; bool const0;
		int pos1; bool const1;
		int pos2; bool const2;

		static IndexedTriangle Create(int p0, int p1, int p2, const Vector<IndexedEdge>& constraints);

		Triangle Convert(DelaunayGenerator* pGen) const;
		Vector3 GetGravity(DelaunayGenerator* pGen) const;
		Vector3 Convert(int index, DelaunayGenerator* pGen) const;

		bool IsSame(const IndexedTriangle& tri) const
		{
			if (pos0 == tri.pos0 && pos1 == tri.pos1 && pos2 == tri.pos2) { return true; }
			if (pos0 == tri.pos0 && pos1 == tri.pos2 && pos2 == tri.pos1) { return true; }
			
			if (pos0 == tri.pos1 && pos1 == tri.pos0 && pos2 == tri.pos2) { return true; }
			if (pos0 == tri.pos1 && pos1 == tri.pos2 && pos2 == tri.pos0) { return true; }
			
			if (pos0 == tri.pos2 && pos1 == tri.pos1 && pos2 == tri.pos0) { return true; }
			if (pos0 == tri.pos2 && pos1 == tri.pos0 && pos2 == tri.pos1) { return true; }

			return false;
		}
		bool HasConstraint() const
		{
			return const0 || const1 || const2;
		}
		bool HasHuge() const
		{
			return
				pos0 == HUGE_POS0 || pos0 == HUGE_POS1 || pos0 == HUGE_POS2 ||
				pos1 == HUGE_POS0 || pos1 == HUGE_POS1 || pos1 == HUGE_POS2 ||
				pos2 == HUGE_POS0 || pos2 == HUGE_POS1 || pos2 == HUGE_POS2;
		}

		IndexedEdge GetEdge0() const { return IndexedEdge(pos0, pos1); }
		IndexedEdge GetEdge1() const { return IndexedEdge(pos1, pos2); }
		IndexedEdge GetEdge2() const { return IndexedEdge(pos2, pos0); }

		size_t GetHash() const
		{
			return
				std::hash<int>()(pos0) ^
				std::hash<int>()(pos1) << 1 ^
				std::hash<int>()(pos2) << 2;
		}

		struct Hash
		{
			std::size_t operator()(const IndexedTriangle& e) const
			{
				return e.GetHash();
			}
		};
	};

	void Division(const Vector<Vector3>& position, int index);
	void DivisionConstraint(const Vector<Vector3>& position, int index);
	bool InnerByCircle(const DelaunayGenerator::Circumscribe& circle, const Vector3& point);
	Circumscribe CalcCircumscribedCircle(const Triangle& triangle);
	Circumscribe CalcCircumscribedCircle(const IndexedTriangle& triangle);
	Triangle CreateHugeTriangle(const Vector<Vector3>& position);
	void RemoveHugeTriangle();
	
	UI m_ui;
	const Vector<Vector3>* m_target;
	Vector<const Vector<Vector3>*> m_inner;
	std::unordered_set<IndexedEdge,IndexedEdge::Hash> m_ConstraintEdge;
	Triangle m_HugeTriangle;
	List<IndexedTriangle> m_Delaunay;
	bool m_ccw;
};

class HalfEdgeNode;
class Delaunay3DGenerator : public IAlgorithm
{
	struct Tetrahedron
	{
		Vector3 p0;
		Vector3 p1;
		Vector3 p2;
		Vector3 p3;
	};

	struct Circumsphere
	{
		Circumsphere():radius(0.0f) {}
		Circumsphere(const Vector3& p, float r)
			: position(p)
			, radius(r) {}
		float radius;
		Vector3 position;
	};

public:
	Delaunay3DGenerator() {};
	~Delaunay3DGenerator() {};
	ALGORITHM_TYPE GetType() { return ALGORITHM_DELAUNAY_3D; }
	Circumsphere CreateCircumsphere(const Tetrahedron& tet);
	Tetrahedron CreateHugeTetrahedron(const BDB& bdb) const;

private:
	Tetrahedron m_HugeTetrahedron;
	Vector<Tetrahedron> m_Delaunay;
};


}

#endif DELAUNAY_GENERATOR

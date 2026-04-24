#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H
namespace KI
{
class BDB
{
public:
	BDB();
	BDB(const Vector<Vector3>& value);
	BDB(const Vector3& min, const Vector3& max);
	~BDB();
	bool IsActive() const;
	void Add(const Vector3& pos);
	void Add(const BDB& box);
	void Set(Vector3 min, Vector3 max);
	void Apply(const Vector3& position);
	void Apply(const BDB& bdb);
	const Vector3& Min() const { return m_min; }
	const Vector3& Max() const { return m_max; }
	float MaxLength() const;
	float XLength() const { return m_max.x - m_min.x; }
	float YLength() const { return m_max.y - m_min.y; }
	float ZLength() const { return m_max.z - m_min.z; }
	Vector3 Center() const;


	struct Triangle
	{
		Triangle() {}
		Triangle(const Vector3& p0, const Vector3& p1, const Vector3& p2)
			:pos0(p0), pos1(p1), pos2(p2) {}
		Vector3 pos0;
		Vector3 pos1;
		Vector3 pos2;
	};

	std::array<Vector3, 8> CreateBoxPos() const;
	std::array<Vector3, 24> CreateLine() const;
	std::array<Triangle, 12> CreateTriangle() const;
	BDB CreateRotate(const Matrix4x4& matrix) const;
private:
	Vector3 m_min;
	Vector3 m_max;
	
};

template<typename T>
struct Range
{
public:
	bool IsIn(T v) const { return v >= min && v <= max; }
	T Length() const { return max - min; }
	void Add(T v)
	{
		min = std::min(min, v);
		max = std::max(max, v);
	}

	T Min() const { return min; }
	T Max() const { return max; }
private:
	T min = std::numeric_limits<T>::max();
	T max = std::numeric_limits<T>::lowest();
};

typedef Range<int> Rangei;	

struct Rangef : public Range<float>
{
public:
	int RoundMin() const { return (int)std::round(Min()); }
	int RoundMax() const { return (int)std::round(Max()); }
};
}
#endif BOUNDING_BOX_H
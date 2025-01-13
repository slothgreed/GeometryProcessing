#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H
namespace KI
{
class BDB
{
public:
	BDB();
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
	Vector3 Center() const;

	BDB CreateRotate(const Matrix4x4& matrix) const;
private:
	Vector3 m_min;
	Vector3 m_max;
	
};
}
#endif BOUNDING_BOX_H
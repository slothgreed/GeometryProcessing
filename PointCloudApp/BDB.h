#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H
class BDB
{
public:
	BDB();
	BDB(vec3 min, vec3 max);
	~BDB();
	void Add(const vec3& pos);
	void Set(vec3 min, vec3 max);
	void Apply(const vec3& position);
	void Apply(const BDB& bdb);
	const vec3& Min() const { return m_min; }
	const vec3& Max() const { return m_max; }
	const vec3& Center() const { return m_center; }

private:
	vec3 m_min;
	vec3 m_max;
	vec3 m_center;
	
};
#endif BOUNDING_BOX_H
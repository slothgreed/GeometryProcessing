#include "BDB.h"
#include <algorithm>
using namespace std;
BDB::BDB()
{
	Set(vec3(numeric_limits<float>::infinity()),
		vec3(-numeric_limits<float>::infinity()));
}

BDB::~BDB()
{
}

BDB::BDB(const vec3& min, const vec3& max)
{
	Set(min, max);
}
bool BDB::IsActive() const
{
	return m_min.x != numeric_limits<float>::infinity();
}
void BDB::Apply(const vec3& position)
{
	m_min.x = glm::min(m_min.x, position.x);
	m_min.y = glm::min(m_min.y, position.y);
	m_min.z = glm::min(m_min.z, position.z);

	m_max.x = glm::max(m_max.x, position.x);
	m_max.y = glm::max(m_max.y, position.y);
	m_max.z = glm::max(m_max.z, position.z);
	Set(m_min, m_max);
}

void BDB::Apply(const BDB& bdb)
{
	m_min.x = glm::min(m_min.x, bdb.Min().x);
	m_min.y = glm::min(m_min.y, bdb.Min().y);
	m_min.z = glm::min(m_min.z, bdb.Min().z);


	m_max.x = glm::max(m_max.x, bdb.Max().x);
	m_max.y = glm::max(m_max.y, bdb.Max().y);
	m_max.z = glm::max(m_max.z, bdb.Max().z);

	Set(m_min, m_max);
}

void BDB::Add(const vec3& pos)
{
	if (pos.x < m_min.x) { m_min.x = pos.x; }
	if (pos.y < m_min.y) { m_min.y = pos.y; }
	if (pos.z < m_min.z) { m_min.z = pos.z; }

	if (pos.x > m_max.x) { m_max.x = pos.x; }
	if (pos.y > m_max.y) { m_max.y = pos.y; }
	if (pos.z > m_max.z) { m_max.z = pos.z; }
	
	m_center = vec3(
		(m_min.x + m_max.x) * 0.5,
		(m_min.y + m_max.y) * 0.5,
		(m_min.z + m_max.z) * 0.5
	);
}
void BDB::Set(vec3 min, vec3 max)
{
	m_min = min;
	m_max = max;
	m_center = vec3(
		(m_min.x + m_max.x) * 0.5,
		(m_min.y + m_max.y) * 0.5,
		(m_min.z + m_max.z) * 0.5
	);

}


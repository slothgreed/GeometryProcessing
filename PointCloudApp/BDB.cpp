#include "BDB.h"
#include <algorithm>
using namespace std;
namespace KI
{
BDB::BDB()
{
	Set(Vector3(numeric_limits<float>::infinity()),
		Vector3(-numeric_limits<float>::infinity()));
}

BDB::~BDB()
{
}

BDB::BDB(const Vector3& min, const Vector3& max)
{
	Set(min, max);
}
bool BDB::IsActive() const
{
	return m_min.x != numeric_limits<float>::infinity() &&
		m_min.x != -numeric_limits<float>::infinity();
}
void BDB::Apply(const Vector3& position)
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
	if (!bdb.IsActive()) { return; }
	m_min.x = glm::min(m_min.x, bdb.Min().x);
	m_min.y = glm::min(m_min.y, bdb.Min().y);
	m_min.z = glm::min(m_min.z, bdb.Min().z);


	m_max.x = glm::max(m_max.x, bdb.Max().x);
	m_max.y = glm::max(m_max.y, bdb.Max().y);
	m_max.z = glm::max(m_max.z, bdb.Max().z);

	Set(m_min, m_max);
}

void BDB::Add(const Vector3& pos)
{
	if (pos.x < m_min.x) { m_min.x = pos.x; }
	if (pos.y < m_min.y) { m_min.y = pos.y; }
	if (pos.z < m_min.z) { m_min.z = pos.z; }

	if (pos.x > m_max.x) { m_max.x = pos.x; }
	if (pos.y > m_max.y) { m_max.y = pos.y; }
	if (pos.z > m_max.z) { m_max.z = pos.z; }
}

void BDB::Add(const BDB& box)
{
	if (!box.IsActive()) { return; }
	Add(box.Max());
	Add(box.Min());
}
void BDB::Set(Vector3 min, Vector3 max)
{
	m_min = min;
	m_max = max;
}

float BDB::MaxLength() const
{
	auto length = m_max - m_min;
	return std::max(std::max(length.x,length.y),length.z);
}

Vector3 BDB::Center() const
{
	return Vector3(
		(m_min.x + m_max.x) * 0.5,
		(m_min.y + m_max.y) * 0.5,
		(m_min.z + m_max.z) * 0.5
	);
}

std::array<Vector3, 8> BDB::CreateBoxPos() const
{
	std::array<Vector3, 8> boxPos;
	boxPos[0] = (m_min);
	boxPos[1] = (Vector3(m_max.x, m_min.y, m_min.z));
	boxPos[2] = (Vector3(m_max.x, m_max.y, m_min.z));
	boxPos[3] = (Vector3(m_min.x, m_max.y, m_min.z));
	boxPos[4] = (Vector3(m_min.x, m_min.y, m_max.z));
	boxPos[5] = (Vector3(m_max.x, m_min.y, m_max.z));
	boxPos[6] = (m_max);
	boxPos[7] = (Vector3(m_min.x, m_max.y, m_max.z));
	
	return boxPos;
}
std::array<Vector3, 24> BDB::CreateLine() const
{
	std::array<Vector3, 24> lines;
	auto p = CreateBoxPos();

	int index = 0;
	lines[index++] = p[0]; lines[index++] = p[1];
	lines[index++] = p[1]; lines[index++] = p[2];
	lines[index++] = p[2]; lines[index++] = p[3];
	lines[index++] = p[3]; lines[index++] = p[0];
	
	lines[index++] = p[4]; lines[index++] = p[5];
	lines[index++] = p[5]; lines[index++] = p[6];
	lines[index++] = p[6]; lines[index++] = p[7];
	lines[index++] = p[7]; lines[index++] = p[4];
	
	lines[index++] = p[0]; lines[index++] = p[4];
	lines[index++] = p[1]; lines[index++] = p[5];
	lines[index++] = p[2]; lines[index++] = p[6];
	lines[index++] = p[3]; lines[index++] = p[7];

	return lines;
}
std::array<BDB::Triangle, 12> BDB::CreateTriangle() const
{
	std::array<BDB::Triangle, 12> tri;
	auto p = CreateBoxPos();

	int index = 0;
	tri[index++] = Triangle(p[0], p[3], p[2]);
	tri[index++] = Triangle(p[0], p[2], p[1]);

	tri[index++] = Triangle(p[0], p[4], p[7]);
	tri[index++] = Triangle(p[0], p[7], p[3]);
	
	tri[index++] = Triangle(p[4], p[5], p[6]);
	tri[index++] = Triangle(p[4], p[6], p[7]);
	
	tri[index++] = Triangle(p[1], p[2], p[6]);
	tri[index++] = Triangle(p[1], p[6], p[5]);
	
	tri[index++] = Triangle(p[2], p[3], p[7]);
	tri[index++] = Triangle(p[2], p[7], p[6]);
	
	tri[index++] = Triangle(p[1], p[5], p[4]);
	tri[index++] = Triangle(p[1], p[4], p[0]);

	return tri;
}


BDB BDB::CreateRotate(const Matrix4x4& matrix) const
{
	BDB bdb;
	bdb.m_min = matrix * Vector4(m_min, 1.0f);
	bdb.m_max = matrix * Vector4(m_max, 1.0f);
	return bdb;
}
}


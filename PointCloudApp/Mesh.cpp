#include "Mesh.h"
#include "Utility.h"
namespace KI
{
int Mesh::TriangleNum() const
{
	if (m_drawType == DrawType::Triangles) {
		if (m_indexs.size() != 0) {
			return m_indexs.size() / 3;
		} else {
			return m_points.size() / 3;
		}
	} else if (
		m_drawType == DrawType::TriangleFan ||
		m_drawType == DrawType::TriangleStrip) {
		if (m_indexs.size() != 0) {
			return m_indexs.size() - 2;
		} else {
			return m_points.size() - 2;
		}
	}

	return 0;
}

Mesh::Triangle Mesh::GetTriangle(int index) const
{
	Triangle tri;
	if (m_drawType == DrawType::Triangles) {
		if (m_indexs.empty()) {
			tri.pos0 = m_points[3 * index + 0];
			tri.pos1 = m_points[3 * index + 1];
			tri.pos2 = m_points[3 * index + 2];
			return tri;
		} else {
			tri.pos0 = m_points[m_indexs[3 * index + 0]];
			tri.pos1 = m_points[m_indexs[3 * index + 1]];
			tri.pos2 = m_points[m_indexs[3 * index + 2]];
			return tri;
		}
	} else {
		Assert::Failed();
		return tri;
	}
}

void Mesh::BuildNormal()
{
	if (m_drawType == DrawType::Triangles) {
		if (m_indexs.empty()) { Assert::Failed(); }
		m_normals.resize(m_points.size());
		for (UInt i = 0; i < m_indexs.size(); i += 3) {
			auto p0 = m_points[m_indexs[i]];
			auto p1 = m_points[m_indexs[i + 1]];
			auto p2 = m_points[m_indexs[i + 2]];
			auto faceNormal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
			m_normals[m_indexs[i]] = faceNormal;
			m_normals[m_indexs[i + 1]] = faceNormal;
			m_normals[m_indexs[i + 2]] = faceNormal;
		}
	}
}

void Mesh::Normalize()
{
	if (m_points.empty()) {	return;	}

	BDB bdb(m_points);

	const Vector3 center = bdb.Center();
	const float scale = bdb.MaxLength();
	if (scale <= std::numeric_limits<float>::epsilon()) {
		return;
	}

	for (auto& point : m_points) {
		point = (point - center) / scale;
	}
}

void Mesh::RemoveArea0Trianlge()
{
	if (m_drawType != DrawType::Triangles) {
		Assert::Failed();
		return;
	}

	constexpr float epsilon = 1e-12f;
	auto isArea0 = [](const Vector3& p0, const Vector3& p1, const Vector3& p2)
	{
		const Vector3 e0 = p1 - p0;
		const Vector3 e1 = p2 - p0;
		const Vector3 cross = glm::cross(e0, e1);
		return glm::dot(cross, cross) <= epsilon;
	};

	//---------------------------------------------------------------------
	// Indexed Triangle
	//---------------------------------------------------------------------
	if (!m_indexs.empty()) {
		if (m_indexs.size() % 3 != 0) { Assert::Failed();			return; }

		Vector<UInt> newIndexs;
		newIndexs.reserve(m_indexs.size());

		for (size_t i = 0; i < m_indexs.size(); i += 3) {

			const UInt i0 = m_indexs[i + 0];
			const UInt i1 = m_indexs[i + 1];
			const UInt i2 = m_indexs[i + 2];

			if (i0 >= m_points.size() ||
				i1 >= m_points.size() ||
				i2 >= m_points.size()) {
				Assert::Failed();
				return;
			}

			const auto& p0 = m_points[i0];
			const auto& p1 = m_points[i1];
			const auto& p2 = m_points[i2];

			if (isArea0(p0, p1, p2)) {
				continue;
			}

			newIndexs.push_back(i0);
			newIndexs.push_back(i1);
			newIndexs.push_back(i2);
		}

		m_indexs = std::move(newIndexs);

		//-----------------------------------------------------------------
		// TriangleçÌèúÇ…ÇÊÇ¡Çƒñ¢égópVertexÇ™î≠ê∂Ç∑ÇÈâ¬î\ê´Ç™Ç†ÇÈ
		//-----------------------------------------------------------------
		RemoveUnusedVertex();
		return;
	}

	//---------------------------------------------------------------------
	// Non Indexed Triangle
	//---------------------------------------------------------------------
	if (m_points.size() % 3 != 0) { Assert::Failed();		return; }

	Vector<Vector3> newPoints;
	newPoints.reserve(m_points.size());

	for (size_t i = 0; i < m_points.size(); i += 3) {

		const auto& p0 = m_points[i + 0];
		const auto& p1 = m_points[i + 1];
		const auto& p2 = m_points[i + 2];

		if (isArea0(p0, p1, p2)) { continue; }

		newPoints.push_back(p0);
		newPoints.push_back(p1);
		newPoints.push_back(p2);
	}

	m_points = std::move(newPoints);
}
void Mesh::RemoveUnusedVertex()
{
	if (m_drawType != DrawType::Triangles) { Assert::Failed(); return; }
	if (m_indexs.empty()) { return; }
	const UInt invalidIndex = std::numeric_limits<UInt>::max();

	Vector<UInt> remap(m_points.size(), invalidIndex);

	UInt newVertexCount = 0;

	for (UInt index : m_indexs) {
		if (index >= m_points.size()) {
			Assert::Failed();
			return;
		}

		if (remap[index] == invalidIndex) {
			remap[index] = newVertexCount;
			++newVertexCount;
		}
	}

	if (newVertexCount == m_points.size()) { return; }

	Vector<Vector3> newPoints;
	newPoints.resize(newVertexCount);

	for (UInt oldIndex = 0; oldIndex < static_cast<UInt>(m_points.size()); ++oldIndex) {
		const UInt newIndex = remap[oldIndex];

		if (newIndex == invalidIndex) { continue; }

		newPoints[newIndex] = m_points[oldIndex];
	}

	for (UInt& index : m_indexs) { index = remap[index]; }

	m_points = std::move(newPoints);
}

Mesh Mesh::CreateIndexedTriangle() const
{
	auto mesh = *this;
	mesh.ConvertIndexedTriangles();
	return mesh;
}
void Mesh::ConvertIndexedTriangles()
{
	if (m_drawType != DrawType::Triangles) { Assert::Failed(); return; }
	if (!m_indexs.empty()) { return; }

	Vector<Vector3> dstPoints;
	Vector<UInt> dstIndexs;
	dstPoints.reserve(m_points.size());
	dstIndexs.reserve(m_points.size());
	struct Vector3Key
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;

		bool operator==(const Vector3Key& rhs) const
		{
			return 
				x == rhs.x &&
				y == rhs.y &&
				z == rhs.z;
		}

		static Vector3Key Create(const Vector3& p)
		{
			return Vector3Key
			{
				std::bit_cast<uint32_t>(p.x),
				std::bit_cast<uint32_t>(p.y),
				std::bit_cast<uint32_t>(p.z)
			};
		};
	};

	struct Vector3Hash
	{
		size_t operator()(const Vector3Key& key) const
		{
			size_t h = std::hash<uint32_t>{}(key.x);
			h ^= std::hash<uint32_t>{}(key.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<uint32_t>{}(key.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};


	std::unordered_map<Vector3Key, UInt, Vector3Hash> vertexMap;
	vertexMap.reserve(m_points.size());
	
	for (const Vector3& point : m_points) {
		const auto key = Vector3Key::Create(point);
		auto it = vertexMap.find(key);
		if (it != vertexMap.end()) {
			dstIndexs.push_back(it->second);
			continue;
		}

		const UInt index = static_cast<UInt>(dstPoints.size());
		dstPoints.push_back(point);
		dstIndexs.push_back(index);
		vertexMap.emplace(key, index);
	}

	m_points = std::move(dstPoints);
	m_indexs = std::move(dstIndexs);
}

Mesh& Mesh::ConvertTriangles()
{
	if (m_drawType == DrawType::Triangles) {
		if (m_indexs.empty()) { return *this; }
		Vector<Vector3> points(m_indexs.size());
		for (size_t i = 0; i < m_indexs.size(); i++) {
			points[i] = m_points[m_indexs[i]];
		}
		m_points = std::move(points);
		m_indexs.clear();
	} else {
		Assert::Failed();
	}

	return *this;
}

Mesh& Mesh::Reverse()
{
	if (m_drawType == DrawType::Triangles) {
		if (m_indexs.empty()) {
			Vector<Vector3> points(m_points.size());
			for (size_t i = 0; i < m_points.size(); i += 3) {
				points[i] = m_points[i];
				points[i + 1] = m_points[i + 2];
				points[i + 2] = m_points[i + 1];
			}
			m_points = std::move(points);
		} else {
			Vector<UInt> indexs(m_indexs.size());
			for (size_t i = 0; i < m_indexs.size(); i += 3) {
				indexs[i] = m_indexs[i];
				indexs[i + 1] = m_indexs[i + 2];
				indexs[i + 2] = m_indexs[i + 1];
			}
			m_indexs = std::move(indexs);
		}
	} else {
		Assert::Failed();
	}

	return *this;
}
}
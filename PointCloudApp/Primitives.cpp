#include "Primitives.h"
#include "Utility.h"
#include "KIMath.h"
namespace KI
{
Cube::Cube(const Vector3& min, const Vector3& max)
	:m_min(min), m_max(max)
{
	m_position.resize(8);
	m_position[0] = (m_min);
	m_position[1] = (Vector3(m_max.x, m_min.y, m_min.z));
	m_position[2] = (Vector3(m_max.x, m_max.y, m_min.z));
	m_position[3] = (Vector3(m_min.x, m_max.y, m_min.z));

	m_position[4] = (Vector3(m_min.x, m_min.y, m_max.z));
	m_position[5] = (Vector3(m_max.x, m_min.y, m_max.z));
	m_position[6] = (m_max);
	m_position[7] = (Vector3(m_min.x, m_max.y, m_max.z));

	m_index.clear();
	AddTriangle(m_index, 0, 3, 2, 1);
	AddTriangle(m_index, 0, 4, 7, 3);
	AddTriangle(m_index, 4, 5, 6, 7);
	AddTriangle(m_index, 1, 2, 6, 5);
	AddTriangle(m_index, 2, 3, 7, 6);
	AddTriangle(m_index, 1, 5, 4, 0);


	CalcNormal();
	m_primitiveType = GL_TRIANGLES;
}

Cube Cube::CreateLine(const Vector3& min, const Vector3& max)
{
	Cube cube;
	cube.m_min = min; cube.m_max = max;
	cube.m_position.resize(8);
	cube.m_position[0] = (min);
	cube.m_position[1] = (Vector3(max.x, min.y, min.z));
	cube.m_position[2] = (Vector3(max.x, max.y, min.z));
	cube.m_position[3] = (Vector3(min.x, max.y, min.z));
	cube.m_position[4] = (Vector3(min.x, min.y, max.z));
	cube.m_position[5] = (Vector3(max.x, min.y, max.z));
	cube.m_position[6] = (max);
	cube.m_position[7] = (Vector3(min.x, max.y, max.z));

	cube.m_index.clear();
	cube.m_index.push_back(0); cube.m_index.push_back(1);
	cube.m_index.push_back(1); cube.m_index.push_back(2);
	cube.m_index.push_back(2); cube.m_index.push_back(3);
	cube.m_index.push_back(3); cube.m_index.push_back(0);


	cube.m_index.push_back(4); cube.m_index.push_back(5);
	cube.m_index.push_back(5); cube.m_index.push_back(6);
	cube.m_index.push_back(6); cube.m_index.push_back(7);
	cube.m_index.push_back(7); cube.m_index.push_back(4);



	cube.m_index.push_back(0); cube.m_index.push_back(4);
	cube.m_index.push_back(1); cube.m_index.push_back(5);
	cube.m_index.push_back(2); cube.m_index.push_back(6);
	cube.m_index.push_back(3); cube.m_index.push_back(7);

	cube.m_primitiveType = GL_LINES;
	return cube;
}

PlanePrimitive::PlanePrimitive(const Vector3& min, const Vector3& max, float position, Axis axis, bool texcoord)
{
	if (axis == Axis::X) {
		m_position.push_back(Vector3(position, min.y, min.z));
		m_position.push_back(Vector3(position, min.y, max.z));
		m_position.push_back(Vector3(position, max.y, max.z));
		m_position.push_back(Vector3(position, max.y, min.z));
		if (texcoord) {
			m_texcoord.resize(4);
			m_texcoord[0] = vec2(0, 0.0);
			m_texcoord[1] = vec2(1.0, 0.0);
			m_texcoord[2] = vec2(1.0, 1.0);
			m_texcoord[3] = vec2(0, 1.0);
		}
	} else if (axis == Axis::Y) {
		m_position.push_back(Vector3(min.x, position, min.z));
		m_position.push_back(Vector3(min.x, position, max.z));
		m_position.push_back(Vector3(max.x, position, max.z));
		m_position.push_back(Vector3(max.x, position, min.z));
		if (texcoord) {
			m_texcoord.resize(4);
			m_texcoord[0] = vec2(0, 0.0);
			m_texcoord[1] = vec2(0, 1.0);
			m_texcoord[2] = vec2(1.0, 1.0);
			m_texcoord[3] = vec2(1.0, 0.0);
		}
	} else if (axis == Axis::Z) {
		m_position.push_back(Vector3(min.x, min.y, position));
		m_position.push_back(Vector3(max.x, min.y, position));
		m_position.push_back(Vector3(max.x, max.y, position));
		m_position.push_back(Vector3(min.x, max.y, position));
		if (texcoord) {
			m_texcoord.resize(4);
			m_texcoord[0] = vec2(0, 0.0);
			m_texcoord[1] = vec2(0, 1.0);
			m_texcoord[2] = vec2(1.0, 1.0);
			m_texcoord[3] = vec2(1.0, 0.0);
		}
	}

	m_index.push_back(0); m_index.push_back(1); m_index.push_back(2);
	m_index.push_back(2); m_index.push_back(3); m_index.push_back(0);
	m_primitiveType = GL_TRIANGLES;
}

Matrix4x4 PlanePrimitive::CreateMatrix(const Vector3& min, const Vector3& max, float position, Axis axis)
{
	// Step 1: 幅と高さ（他の2軸）を求める
	switch (axis) {
	case Axis::X:
		return glmUtil::CreateTranslate(Vector3(position, 0.0f, 0.0f)); // XY→YZ面へ
	case Axis::Y:
		return glmUtil::CreateTranslate(Vector3(0.0f, position, 0.0f)) * glmUtil::CreateRotateAngle(Vector3(0, 0, -90)); // XY→XZ面へ
	case Axis::Z:
		return glmUtil::CreateTranslate(Vector3(0.0f, 0.0f, position)) * glmUtil::CreateRotateAngle(Vector3(0, 90, 0)); // そのままXY平面
	}

	return Matrix4x4();
}

Cone::Cone(float _radius, float _height, int _partition)
	: radius(_radius)
	, height(_height)
	, partition(_partition)
{
	m_position.push_back(Vector3(0, height, 0));
	m_position.push_back(Vector3(0, 0, 0));

	float rad = 2 * pi<float>() / partition;
	float theta = 0;

	float y = pi<float>() / 2;
	for (int i = 0; i <= partition; i++)
	{
		float x = -(i * rad);
		m_position.push_back(Vector3(
			radius * sin(y) * cos(x),
			radius * cos(y),
			radius * sin(y) * sin(x)
		));
	}

	int index1;
	int index2;
	for (int i = 0; i <= partition; i++)
	{

		index1 = 2 + i;
		if (2 + i + 1 == m_position.size())
		{
			index2 = 2;
		}
		else
		{
			index2 = 2 + i + 1;
		}

		m_index.push_back(0);			// 先端点
		m_index.push_back(index1);
		m_index.push_back(index2);

		// 終端を結ぶ
		m_index.push_back(1);			// 底面の中心点
		m_index.push_back(index2);
		m_index.push_back(index1);
	}

	CalcNormal();
	m_primitiveType = GL_TRIANGLES;
}

Vector3 Cone::CalcApex(const Vector3& center, const Vector3& axis, float radius, float semiAngleDeg)
{
	float tanAlpha = std::tan(MathHelper::ToRadian(semiAngleDeg));

	// tan(alpha)=0 は円柱扱いなのでガード
	if (MathHelper::IsZero(tanAlpha)) {	return center;}

	return center + glm::normalize(axis) * (-radius / tanAlpha);
}
Mesh Cone::CreateSideMesh(
	const Vector3& baseCenter, const Vector3& axis,
	const Vector3& beginPoint, const Vector3& endPoint,
	float radius, float height, float semiAngleDeg, bool orient, int slices, int stacks)
{
	Vector<Vector3> position;
	Vector<UInt> indices;
	slices = std::max(3, slices);
	stacks = std::max(1, stacks);

	// -----------------------------
	// 軸方向
	// -----------------------------
	Vector3 z = glm::normalize(axis);

	// -----------------------------
	// beginPoint を基準に局所座標系構築
	// x: 円周方向の基準
	// y: x と z に直交するもう一方
	// -----------------------------
	Vector3 beginDir = beginPoint - baseCenter;
	beginDir = beginDir - z * glm::dot(beginDir, z);

	// 万一 beginDir が軸に近すぎる場合の保険
	if (MathHelper::IsZero(beginDir)) {
		Vector3 fallback = (std::abs(z.z) < 0.999f) ? Vector3(0, 0, 1) : Vector3(1, 0, 0);
		beginDir = glm::cross(fallback, z);
	}

	Vector3 x = glm::normalize(beginDir);
	Vector3 y = glm::normalize(glm::cross(z, x));

	// -----------------------------
	// end angle の算出
	// beginPoint を角度0とみなす
	// -----------------------------
	auto closed = MathHelper::IsSame(beginPoint, endPoint);
	float endAngle = MathHelper::PI2;
	if (!closed) {
		Vector3 d = endPoint - baseCenter;
		d = d - z * glm::dot(d, z);
		if (MathHelper::IsZero(glm::length2(d))) {
			endAngle = 0.0f;
		} else {
			d = glm::normalize(d);
			endAngle = MathHelper::ToRadian(d, x, y, 1);
			endAngle = MathHelper::NormalizePI(endAngle);
		}
	}

	// -----------------------------
	// 半頂角 -> 半径変化量
	// r(h) = radius - h * tan(alpha)
	// -----------------------------
	float tanAlpha = std::tan(MathHelper::ToRadian(semiAngleDeg));
	if (glm::dot(Cone::CalcApex(baseCenter, axis, radius, semiAngleDeg) - baseCenter, axis) < 0) {
		z = -z;
	}

	// -----------------------------
	// 頂点生成
	// apex を含む場合、最上段半径が 0 近傍になることがある
	// その場合も一旦そのまま生成する
	// -----------------------------
	const int ringVertexCount = closed ? slices : (slices + 1);

	for (int iy = 0; iy <= stacks; ++iy) {
		float vTex = static_cast<float>(iy) / static_cast<float>(stacks);
		float h = height * vTex;
		float r = radius - h * tanAlpha;

		// 負半径にはしない
		if (r < 0.0f) r = 0.0f;
		Vector3 center = baseCenter + z * h;

		for (int ix = 0; ix < ringVertexCount; ++ix) {
			float uTex = static_cast<float>(ix) / static_cast<float>(slices);
			float angle = endAngle * uTex;
			Vector3 radial = x * std::cos(angle) + y * std::sin(angle);
			position.push_back(center + radial * r);
		}
	}

	// -----------------------------
	// インデックス生成
	// apex 行が潰れていても基本は quad 分割で張る
	// 必要に応じて退化三角形をスキップ
	// -----------------------------
	auto AddTriangle = [&](uint32_t a, uint32_t b, uint32_t c)
	{
		const Vector3& p0 = position[a];
		const Vector3& p1 = position[b];
		const Vector3& p2 = position[c];

		Vector3 e0 = p1 - p0;
		Vector3 e1 = p2 - p0;
		if (MathHelper::IsZero(glm::cross(e0, e1)))	return; // 退化三角形は捨てる

		if (!orient) {
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		} else {
			indices.push_back(a);
			indices.push_back(c);
			indices.push_back(b);
		}
	};

	for (int iy = 0; iy < stacks; ++iy) {
		int row0 = iy * ringVertexCount;
		int row1 = (iy + 1) * ringVertexCount;

		for (int ix = 0; ix < slices; ++ix) {
			int i0 = row0 + ix;
			int i1 = row0 + ((ix + 1) % ringVertexCount);
			int i2 = row1 + ix;
			int i3 = row1 + ((ix + 1) % ringVertexCount);

			if (!closed) {
				i1 = row0 + (ix + 1);
				i3 = row1 + (ix + 1);
			}

			AddTriangle(i0, i2, i1);
			AddTriangle(i1, i2, i3);
		}
	}

	return Mesh(std::move(position), std::move(indices), Mesh::DrawType::Triangles);
}

Cylinder::Cylinder(float _baseRad, float _topRad, float _height, int _slices)
	: baseRad(_baseRad)
	, topRad(_topRad)
	, height(_height)
	, slices(_slices)
{
	m_position.push_back(Vector3(0, 0, 0));
	m_position.push_back(Vector3(0, height, 0));

	float sliceStep = 2 * glm::pi<float>() / slices;
	float angle = 0;
	for (int i = 0; i < slices; i++)
	{
		angle = i * sliceStep;
		float cosAngle = cos(angle);
		float sinAngle = sin(angle);
		float xPos = cosAngle * baseRad;
		float yPos = 0;
		float zPos = sinAngle * baseRad;
		m_position.push_back(Vector3(xPos, yPos, zPos));

		xPos = cosAngle * topRad;
		yPos = height;
		zPos = sinAngle * topRad;
		m_position.push_back(Vector3(xPos, yPos, zPos));
	}

	// 底面頂点 => 上面頂点の順で作成
	// k = 0 < 上面・底面部;
	int k = 2;
	for (int i = 0; i < slices - 1; i++)
	{
		// 底面
		m_index.push_back(0);
		m_index.push_back(k);
		m_index.push_back(k + 2);

		// 上面
		m_index.push_back(1);
		m_index.push_back(k + 3);
		m_index.push_back(k + 1);

		//// 側面
		m_index.push_back(k);
		m_index.push_back(k + 1);
		m_index.push_back(k + 2);

		m_index.push_back(k + 2);
		m_index.push_back(k + 1);
		m_index.push_back(k + 3);
		k += 2;
	}

	// 底面
	m_index.push_back(0);
	m_index.push_back(k);
	m_index.push_back(2);

	// 上面
	m_index.push_back(1);
	m_index.push_back(3);
	m_index.push_back(k + 1);

	//// 側面
	m_index.push_back(k);
	m_index.push_back(k + 1);
	m_index.push_back(2);

	m_index.push_back(k + 1);
	m_index.push_back(3);
	m_index.push_back(2);

	CalcNormal();

	m_primitiveType = GL_TRIANGLES;
}

static constexpr float PI = 3.14159265358979323846f;
static inline float clamp01(float a) { return std::min(1.0f, std::max(0.0f, a)); }
static inline float wrap01(float a)
{
	// [0,1) に包む（負もOK）
	a = a - std::floor(a);
	// a==1.0 になり得るケースを避けるなら少しだけ押し戻すが、基本不要
	return a;
}

Vector2 Cylinder::UVConverter::toUV(const Vector3& xyz) const
{
	// v: 高さ方向
	const float invH = 1.0f / m_height;
	float v = xyz.z + (m_height * 0.5f)/ m_height;
	v = clamp01(v);

	// u: 周方向（角度）
	// theta: [-pi, pi]、+Xが0
	float theta = std::atan2(xyz.y, xyz.x);
	// [0, 2pi)
	if (theta < 0.0f) theta += 2.0f * PI;
	float u = theta / (2.0f * PI); // [0,1)
	u = wrap01(u);

	return Vector2(u, v);
}
Vector3 Cylinder::UVConverter::toXYZ(const Vector2& uv) const
{
	// u wrap, v clamp
	const float u = wrap01(uv.x);
	const float v = clamp01(uv.y);


	// 半径は高さに沿って線形補間（円錐台）
	const float r = m_baseRad + (m_topRad - m_baseRad) * v;

	const float theta = u * (2.0f * PI);
	const float x = r * std::cos(theta);
	const float y = v * m_height;
	const float z = r * std::sin(theta);

	return Vector3(x, z, y);
}

Mesh Cylinder::CreateSideMesh(const Vector3& baseCenter, const Vector3& axis, const Vector3& beginPoint, const Vector3& endPoint, float radius, float height, int slices, int stacks)
{
	return CreateSideMesh(baseCenter, axis, beginPoint, endPoint, radius, height, true, slices, stacks);
}
Mesh Cylinder::CreateSideMesh(const Vector3& baseCenter, const Vector3& axis, const Vector3& beginPoint, const Vector3& endPoint, float radius, float height, bool orient, int slices, int stacks)
{
	slices = std::max(3, slices);
	stacks = std::max(1, stacks);

	Vector3 axisPoint = baseCenter + axis * dot(beginPoint - baseCenter, axis);
	Vector3 outward = normalize(beginPoint - axisPoint); // 半径方向
	Vector3 faceNormal = orient ? outward : -outward;

	// 軸方向
	Vector3 z = normalize(axis);

	// begin を基準にローカル座標系構築
	Vector3 beginDir = beginPoint - baseCenter;
	beginDir = beginDir - z * dot(beginDir, z);
	Vector3 x = normalize(beginDir);
	Vector3 y = normalize(cross(z, x));

	bool closed = MathHelper::IsSame(beginPoint, endPoint);
	float endAngle = glm::two_pi<float>();;
	if (!closed) {
		// end の角度
		Vector3 endDir = endPoint - baseCenter;
		endDir = endDir - z * dot(endDir, z);
		endDir = normalize(endDir);
		float cosTheta = dot(endDir, x);
		float sinTheta = dot(endDir, y);
		endAngle = MathHelper::NormalizePI(std::atan2(sinTheta, cosTheta));
	}

	float beginAngle = 0.0f;

	// 列数は閉じないので +1 不要
	const int vertCols = slices + 1;
	const int vertRows = stacks + 1;

	Vector<Vector3> positions;
	Vector<UInt> indices;

	positions.reserve(vertCols * vertRows);
	indices.reserve(slices * stacks * 6);

	// 頂点生成
	for (int iy = 0; iy < vertRows; ++iy) {
		float v = static_cast<float>(iy) / static_cast<float>(stacks);
		float h = v * height;
		Vector3 center = baseCenter + z * h;

		for (int ix = 0; ix < vertCols; ++ix) {
			float u = static_cast<float>(ix) / static_cast<float>(slices);
			float delta = endAngle - beginAngle;
			float theta = beginAngle + u * delta;
			Vector3 radial = std::cos(theta) * x + std::sin(theta) * y;
			Vector3 p = center + radial * radius;
			positions.push_back(p);
		}
	}

	// インデックス
	auto indexOf = [vertCols](int row, int col) -> UInt
	{
		return static_cast<UInt>(row * vertCols + col);
	};

	orient = true;
	for (int iy = 0; iy < stacks; ++iy) {
		for (int ix = 0; ix < slices; ++ix) {
			UInt i0 = indexOf(iy, ix);
			UInt i1 = indexOf(iy, ix + 1);
			UInt i2 = indexOf(iy + 1, ix);
			UInt i3 = indexOf(iy + 1, ix + 1);

			if (ix == 0 && iy == 0) {
				auto normal = MathHelper::CalcNormal(positions[i0], positions[i1], positions[i2]);
				if (glm::dot(normal, faceNormal) < 0) { orient = false; }
			}

			if (orient)	{
				indices.push_back(i0);
				indices.push_back(i1);
				indices.push_back(i2);

				indices.push_back(i1);
				indices.push_back(i3);
				indices.push_back(i2);
			} else {
				indices.push_back(i0);
				indices.push_back(i2);
				indices.push_back(i1);

				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
			}
		}
	}

	return Mesh(std::move(positions), std::move(indices), Mesh::DrawType::Triangles);
}

Polyline Cylinder::CreatePolyline(const Vector3& baseCenter, const Vector3& axis, float radius, float height, int slices, int stacks)
{
	// 軸から直交基底を構築
	auto Z = glm::normalize(axis);
	auto tmp = (fabs(Z.x) < 0.9f) ? Vector3(1, 0, 0) : Vector3(0, 1, 0);
	auto X = glm::normalize(glm::cross(tmp, Z));
	auto Y = glm::cross(Z, X);

	// グリッド状に頂点を計算
	Vector<Vector3> top(slices);
	Vector<Vector3> bottom(slices);
	for (int i = 0; i < slices; ++i) {
		float u = (float)i / slices * glm::two_pi<float>();
		auto fCos = cos(u);
		auto fSin = sin(u);
		auto pos = baseCenter + radius * (fCos * X + fSin * Y);

		bottom[i] = (pos);
		top[slices - i - 1] = (pos + height * Z);
	}

	Vector<unsigned int> lines;
	
	lines.push_back(2 * slices - 1);
	lines.push_back(0);
	// bottom
	{	
		for (size_t i = 0; i < bottom.size() - 1; i++) {
			lines.push_back(i);	lines.push_back(i + 1);
		}

		lines.push_back(slices - 1);
		lines.push_back(0);
	}

	// top
	lines.push_back(0);
	lines.push_back(2 * slices - 1);
	{
		for (size_t i = 0; i < slices - 1; i++) {
			lines.push_back(slices + i);	lines.push_back(slices + i + 1);
		}

		lines.push_back(2 * slices - 1);
		lines.push_back(slices);

	}
	STLUtil::Insert(top, bottom);
	return Polyline(std::move(top), std::move(lines), Polyline::DrawType::Lines);
}


Sphere::Sphere(float _radius, int _slices, int _stacks)
	: radius(_radius)
	, slices(_slices)
	, stacks(_stacks)
{
	using namespace glm;
	float sliceStep = 2 * pi<float>() / slices;
	float stackStep = pi<float>() / stacks;
	float lengthInvert = 1.0f / radius;

	for (int i = 0; i <= stacks; i++)
	{
		float stackAngle = pi<float>() / 2 - i * stackStep;
		float xy = radius * cosf(stackAngle);

		for (int j = 0; j <= slices; j++)
		{
			float sectorAngle = j * sliceStep;
			m_position.push_back(Vector3(
				xy * cosf(sectorAngle),
				xy * sinf(sectorAngle),
				radius * sinf(stackAngle)));

			m_normal.push_back(Vector3(
				m_position[m_position.size() - 1].x * lengthInvert,
				m_position[m_position.size() - 1].y * lengthInvert,
				m_position[m_position.size() - 1].z * lengthInvert));
		}
	}

	for (int i = 0; i < stacks; i++)
	{
		int k1 = i * (slices + 1);
		int k2 = k1 + slices + 1;

		for (int j = 0; j < slices; j++)
		{
			if (i != 0)
			{
				m_index.push_back(k1);
				m_index.push_back(k2);
				m_index.push_back(k1 + 1);
			}

			if (i != stacks - 1)
			{
				m_index.push_back(k1 + 1);
				m_index.push_back(k2);
				m_index.push_back(k2 + 1);
			}

			k1++;
			k2++;
		}
	}

	m_primitiveType = GL_TRIANGLES;
}

Torus::Torus(float _inRad, float _outRad, int _nsides, int _rings)
	: inRad(_inRad)
	, outRad(_outRad)
	, nsides(_nsides)
	, rings(_rings)
{
	float sideAngle = pi<float>() * 2 / nsides;
	float ringAngle = pi<float>() * 2 / rings;
	for (int i = 0; i <= nsides; i++)
	{
		float r = sideAngle * i;
		float rr = (float)cos(r);
		float ry = (float)sin(r);

		for (int j = 0; j <= rings; j++)
		{
			float tr = ringAngle * j;
			float tx = (rr * inRad + outRad) * (float)cos(tr);
			float ty = ry * inRad;
			float tz = (rr * inRad + outRad) * (float)sin(tr);
			m_position.push_back(Vector3(tx, ty, tz));

			float rx = rr * (float)cos(tr);
			float rz = rr * (float)sin(tr);
			m_normal.push_back(Vector3(rx, ry, rz));

			if (i != nsides && j != rings) {
				int index = (rings + 1) * i + j;
				m_index.push_back(index);
				m_index.push_back(index + rings + 1);
				m_index.push_back(index + 1);

				m_index.push_back(index + rings + 1);
				m_index.push_back(index + rings + 2);
				m_index.push_back(index + 1);
			}
		}
	}

	m_primitiveType = GL_TRIANGLES;
}


Vector2 Torus::ToUV(const Vector3& center, const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis, float majorRadius, const Vector3& target)
{
	Vector3 d = target - center;

	float px = glm::dot(d, xAxis);
	float py = glm::dot(d, yAxis);
	float pz = glm::dot(d, zAxis);

	// 大円方向の角度
	float u = std::atan2(py, px);

	// 軸から見た平面内距離
	float rho = std::sqrt(px * px + py * py);

	// 小円方向の角度
	float v = std::atan2(pz, rho - majorRadius);

	return Vector2(u, v);
}
Mesh Torus::CreateMesh(
	const Vector3& center, const Vector3& axis, const Vector3& refDirection,
	float majorRadius, float minorRadius, float uBegin, float uEnd, float vBegin, float vEnd,
	bool orient, int uSegments, int vSegments)
{
	Vector<Vector3> positions;
	Vector<UInt> indices;

	uSegments = std::max(3, uSegments);
	vSegments = std::max(3, vSegments);

	if (majorRadius <= 0.0f || minorRadius <= 0.0f) { return Mesh(); }
	Vector3 z = glm::normalize(axis);
	if (MathHelper::IsZero(z)) { return Mesh(); }
	//if (!vDir) { z = -z; }

	// refDirection を z に直交化して X とする
	Vector3 x = refDirection - z * glm::dot(refDirection, z);
	if (MathHelper::IsZero(x)) {
		x = MathHelper::CreatePerpendicular(z);
	} else {
		x = glm::normalize(x);
	}

	Vector3 y = glm::normalize(glm::cross(z, x));
	if (MathHelper::IsZero(y)) { return Mesh(); }

	const int cols = uSegments + 1;
	const int rows = vSegments + 1;

	for (int j = 0; j <= vSegments; ++j) {
		float tv = static_cast<float>(j) / static_cast<float>(vSegments);
		float v = glm::mix(vBegin, vEnd, tv);

		float cv = std::cos(v);
		float sv = std::sin(v);

		for (int i = 0; i <= uSegments; ++i) {
			float tu = static_cast<float>(i) / static_cast<float>(uSegments);
			float u = glm::mix(uBegin, uEnd, tu);

			float cu = std::cos(u);
			float su = std::sin(u);

			// トーラス中心円上の方向
			Vector3 radial = cu * x + su * y;
			// 中心円上の点
			Vector3 circleCenter = center + majorRadius * radial;

			// 小円の法線方向
			auto normal = glm::normalize(cv * radial + sv * z);
			positions.push_back(circleCenter + minorRadius * normal);
		}
	}

	auto indexOf = [cols](int i, int j) -> UInt
	{
		return static_cast<UInt>(j * cols + i);
	};

	for (int j = 0; j < vSegments; ++j) {
		for (int i = 0; i < uSegments; ++i) {
			UInt i0 = indexOf(i + 0, j + 0);
			UInt i1 = indexOf(i + 1, j + 0);
			UInt i2 = indexOf(i + 1, j + 1);
			UInt i3 = indexOf(i + 0, j + 1);

			if (orient) {
				indices.push_back(i0);
				indices.push_back(i1);
				indices.push_back(i2);

				indices.push_back(i0);
				indices.push_back(i2);
				indices.push_back(i3);
			} else {
				indices.push_back(i0);
				indices.push_back(i2);
				indices.push_back(i1);

				indices.push_back(i0);
				indices.push_back(i3);
				indices.push_back(i2);
			}
		}
	}

	return Mesh(std::move(positions), std::move(indices), Mesh::DrawType::Triangles);
}

Triangle::Triangle()
{
	Build();
}

Triangle::~Triangle()
{
}

void Triangle::Build()
{
	m_position.push_back(Vector3(-0.5, -0.5, 0.0));
	m_position.push_back(Vector3(0.5, -0.5, 0.0));
	m_position.push_back(Vector3(0.25, 0.5, 0.0));
	m_normal.push_back(Vector3(0.0, 0.0, 1.0));
	m_normal.push_back(Vector3(0.0, 0.0, 1.0));
	m_normal.push_back(Vector3(0.0, 0.0, 1.0));
	m_index.push_back(0);
	m_index.push_back(1);
	m_index.push_back(2);
	m_primitiveType = GL_TRIANGLES;
}

Axis::Axis(float size)
{
	Build(Vector3(0, 0, 0), Vector3(size, size, size));
}
Axis::Axis(const Vector3& locate, const Vector3& size)
{
	Build(locate, size);
}

Axis::Axis(const Vector3& locate, const Vector3& u, const Vector3& v, const Vector3& axis)
{
	m_position.push_back(locate); m_position.push_back(locate + u);
	m_position.push_back(locate); m_position.push_back(locate + v);
	m_position.push_back(locate); m_position.push_back(locate + axis);


	m_color.push_back(Vector4(1.0, 0.0, 0.0, 1.0));
	m_color.push_back(Vector4(1.0, 0.0, 0.0, 1.0));
	m_color.push_back(Vector4(0.0, 1.0, 0.0, 1.0));
	m_color.push_back(Vector4(0.0, 1.0, 0.0, 1.0));
	m_color.push_back(Vector4(0.0, 0.0, 1.0, 1.0));
	m_color.push_back(Vector4(0.0, 0.0, 1.0, 1.0));

	m_index.push_back(0); m_index.push_back(1);
	m_index.push_back(2); m_index.push_back(3);
	m_index.push_back(4); m_index.push_back(5);

	m_primitiveType = GL_LINES;
}

void Axis::Build(const Vector3& locate, const Vector3& size)
{
	m_position.push_back(locate);
	m_position.push_back(Vector3(locate.x + size.x, locate.y, locate.z));

	m_position.push_back(locate);
	m_position.push_back(Vector3(locate.x, locate.y + size.y, locate.z));

	m_position.push_back(locate);
	m_position.push_back(Vector3(locate.x, locate.y, locate.z + size.z));


	m_color.push_back(Vector4(1.0, 0.0, 0.0, 1.0));
	m_color.push_back(Vector4(1.0, 0.0, 0.0, 1.0));
	m_color.push_back(Vector4(0.0, 1.0, 0.0, 1.0));
	m_color.push_back(Vector4(0.0, 1.0, 0.0, 1.0));
	m_color.push_back(Vector4(0.0, 0.0, 1.0, 1.0));
	m_color.push_back(Vector4(0.0, 0.0, 1.0, 1.0));

	m_index.push_back(0); m_index.push_back(1);
	m_index.push_back(2); m_index.push_back(3);
	m_index.push_back(4); m_index.push_back(5);

	m_primitiveType = GL_LINES;
}

Vector2 Circle::UVConverter::toUV(const Vector3& xyz)
{
	Vector3 d = xyz - m_center;
	double x = dot(d, m_uAxis);
	double y = dot(d, m_vAxis);
	double theta = std::atan2(y, x); // [-pi, pi]
	if (theta < 0) theta += 2.0 * glm::pi<float>();
	return { theta / (2.0 * glm::pi<float>()), 0.0 };
}
Vector3 Circle::UVConverter::toXYZ(const Vector2& uv)
{
	double theta = uv.x * 2.0 * glm::pi<float>();
	return m_center + m_uAxis * (m_radius * std::cosf(theta))
		+ m_vAxis * (m_radius * std::sinf(theta));
}

Circle::Circle(float radius, const Vector3& center)
{
	Build(radius, 360, center);
}

Circle::Circle(float radius, int pointNum)
{
	Build(radius, pointNum, Vector3(0, 0, 0));
}
Circle::~Circle()
{
}

Vector3 Circle::GetPoint(float radius, const Vector3& u, const Vector3& v, const Vector3& center, const Vector3& begin, const Vector3& end, float parameter)
{
	float delta = 0.0f;
	float beginAngle = MathHelper::ToRadian(begin - center, u, v, radius);
	if (MathHelper::IsSame(begin, end)) {
		delta = MathHelper::PI2;
	} else {
		float endAngle = MathHelper::ToRadian(end - center, u, v, radius);
		delta = MathHelper::NormalizePI(endAngle - beginAngle);
	}
	// 円弧を分割して点を生成
	float angle = beginAngle + delta * parameter;
	return Vector3(center + radius * (std::cos(angle) * u + std::sin(angle) * v));
}
Polyline Circle::CreateLine(float radius, int pointNum, const Vector3& u, const Vector3& v, const Vector3& center)
{
	Vector<Vector3> points;
	for (int i = 0; i < pointNum; i++) {
		auto angle0 = (i / (float)pointNum) * MathHelper::PI2;
		points.push_back(center + radius * (cosf(angle0) * u + sinf(angle0) * v));
	}
	points.push_back(center + radius * (cosf(0) * u + sinf(0) * v));

	return Polyline(std::move(points), Polyline::DrawType::LineStrip).ConvertLines();
}

Polyline Circle::CreateArc(float radius, int pointNum, const Vector3& u, const Vector3& v, const Vector3& center, const Vector3& begin, const Vector3& end)
{
	Vector<Vector3> points;
	float beginAngle = MathHelper::ToRadian(begin - center, u, v, radius);
	float endAngle = MathHelper::ToRadian(end - center, u, v, radius);

	float delta = MathHelper::NormalizePI(endAngle - beginAngle);
	// 円弧を分割して点を生成
	for (int i = 0; i < pointNum; i++) {
		float t = i / (float)pointNum;
		float angle = beginAngle + t * delta;
		Vector3 p = center + radius * (std::cos(angle) * u + std::sin(angle) * v);
		points.push_back(p);
	}
	points.push_back(end);
	return Polyline(std::move(points), Polyline::DrawType::LineStrip).ConvertLines();
}
void Circle::Build(float radius, int pointNum, const Vector3& center)
{
	for (int i = 0; i < pointNum; i++) {
		auto angle0 = (i / (float)pointNum) * MathHelper::PI2;
		auto angle1 = ((i + 1) / (float)pointNum) * MathHelper::PI2;
		m_position.push_back(Vector3(
			radius * cosf(angle0),
			radius * sinf(angle0),
			0.0f) + center);

		m_position.push_back(Vector3(
			radius * cosf(angle1),
			radius * sinf(angle1),
			0.0f) + center);
	}

	m_primitiveType = GL_LINES;
}

RenderPlane::RenderPlane()
{
	Build();
}

GLuint RenderPlane::GetPrimitiveType()
{
	return GL_TRIANGLES;
}

void RenderPlane::Build()
{
	m_position.resize(4);
	m_position[0] = Vector3(-1.0, -1.0, 0.0);
	m_position[1] = Vector3(1.0, -1.0, 0.0);
	m_position[2] = Vector3(1.0, 1.0, 0.0);
	m_position[3] = Vector3(-1.0, 1.0, 0.0);

	m_texcoord.resize(4);
	m_texcoord[0] = vec2(0, 0.0);
	m_texcoord[1] = vec2(1.0, 0.0);
	m_texcoord[2] = vec2(1.0, 1.0);
	m_texcoord[3] = vec2(0, 1.0);

	m_index.resize(6);
	m_index[0] = 0;	m_index[1] = 1;	m_index[2] = 2;
	m_index[3] = 0;	m_index[4] = 2;	m_index[5] = 3;

	m_primitiveType = GL_TRIANGLES;
}

SkyBox::SkyBox()
{
	m_position.clear();
	// positions          
	m_position.push_back(vec3(-1.0,  1.0, -1.0));
	m_position.push_back(vec3(-1.0, -1.0, -1.0));
	m_position.push_back(vec3( 1.0, -1.0, -1.0));
	m_position.push_back(vec3( 1.0, -1.0, -1.0));
	m_position.push_back(vec3( 1.0,  1.0, -1.0));
	m_position.push_back(vec3(-1.0,  1.0, -1.0));

	m_position.push_back(vec3(-1.0, -1.0,  1.0));
	m_position.push_back(vec3(-1.0, -1.0, -1.0));
	m_position.push_back(vec3(-1.0,  1.0, -1.0));
	m_position.push_back(vec3(-1.0,  1.0, -1.0));
	m_position.push_back(vec3(-1.0,  1.0,  1.0));
	m_position.push_back(vec3(-1.0, -1.0,  1.0));

	m_position.push_back(vec3(1.0, -1.0, -1.0)); 
	m_position.push_back(vec3(1.0, -1.0,  1.0)); 
	m_position.push_back(vec3(1.0,  1.0,  1.0)); 
	m_position.push_back(vec3(1.0,  1.0,  1.0)); 
	m_position.push_back(vec3(1.0,  1.0, -1.0)); 
	m_position.push_back(vec3(1.0, -1.0, -1.0)); 

	m_position.push_back(vec3(-1.0, -1.0,  1.0));
	m_position.push_back(vec3(-1.0,  1.0,  1.0));
	m_position.push_back(vec3( 1.0,  1.0,  1.0));
	m_position.push_back(vec3( 1.0,  1.0,  1.0));
	m_position.push_back(vec3( 1.0, -1.0,  1.0));
	m_position.push_back(vec3(-1.0, -1.0,  1.0));

	m_position.push_back(vec3(-1.0,  1.0, -1.0));
	m_position.push_back(vec3( 1.0,  1.0, -1.0));
	m_position.push_back(vec3( 1.0,  1.0,  1.0));
	m_position.push_back(vec3( 1.0,  1.0,  1.0));
	m_position.push_back(vec3(-1.0,  1.0,  1.0));
	m_position.push_back(vec3(-1.0,  1.0, -1.0));

	m_position.push_back(vec3(-1.0, -1.0, -1.0));
	m_position.push_back(vec3(-1.0, -1.0,  1.0));
	m_position.push_back(vec3( 1.0, -1.0, -1.0));
	m_position.push_back(vec3( 1.0, -1.0, -1.0));
	m_position.push_back(vec3(-1.0, -1.0,  1.0));
	m_position.push_back(vec3( 1.0, -1.0,  1.0));

	m_primitiveType = GL_TRIANGLES;
}

Shared<Primitive> ToPrimitive(const Polyline& polyline)
{
	auto primitive = std::make_shared<Primitive>();
	primitive->SetPosition(polyline.GetPoints());
	primitive->SetType(GL_LINE_STRIP);
	return primitive;
}
}
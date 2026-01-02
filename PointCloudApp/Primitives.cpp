#include "Primitives.h"
#include "Utility.h"
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

Polyline Cylinder::CreateOuterLine(const Vector3& baseCenter, const Vector3& axis, float radius, float height, int slices, int stacks)
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
	return Polyline(std::move(top), std::move(lines), Polyline::Hint::Lines);
}


Sphere::Sphere(float _radius, int _slices, int _stacks) :
	radius(_radius), slices(_slices), stacks(_stacks)
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
	Build(size);
}

Axis::~Axis()
{
}

void Axis::Build(float size)
{
	m_position.push_back(Vector3(0.0, 0.0, 0.0));
	m_position.push_back(Vector3(size, 0.0, 0.0));

	m_position.push_back(Vector3(0.0, 0.0, 0.0));
	m_position.push_back(Vector3(0.0, size, 0.0));

	m_position.push_back(Vector3(0.0, 0.0, 0.0));
	m_position.push_back(Vector3(0.0, 0.0, size));


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

Vector2 CircleUVConverter::toUV(const Vector3& xyz)
{
	Vector3 d = xyz - m_center;
	double x = dot(d, m_uAxis);
	double y = dot(d, m_vAxis);
	double theta = std::atan2(y, x); // [-pi, pi]
	if (theta < 0) theta += 2.0 * glm::pi<float>();
	return { theta / (2.0 * glm::pi<float>()), 0.0 };
}
Vector3 CircleUVConverter::toXYZ(const Vector2& uv)
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

Polyline Circle::CreateLine(float radius, int pointNum, const Vector3& u, const Vector3& v, const Vector3& center, bool orient)
{
	Vector<Vector3> points;
	if (orient) {
		for (int i = 0; i < pointNum; i++) {
			auto angle0 = (i / (float)pointNum) * 3.14159f * 2.0f;
			points.push_back(center + radius * (cosf(angle0) * u + sinf(angle0) * v));
		}
	} else {
		for (int i = pointNum - 1; i >= 0; i--) {
			auto angle0 = (i / (float)pointNum) * 3.14159f * 2.0f;
			points.push_back(center + radius * (cosf(angle0) * u + sinf(angle0) * v));
		}
	}

	return Polyline(std::move(points));
}

Polyline Circle::CreateArc(float radius, int pointNum, const Vector3& u, const Vector3& v, const Vector3& center, bool orient, const Vector3& begin, const Vector3& end)
{
	Vector<Vector3> points;

	// begin, end の角度を求める
	auto toAngle = [&](const Vector3& p)
	{
		Vector3 d = p - center;
		float x = glm::dot(d, u) / radius;
		float y = glm::dot(d, v) / radius;
		return std::atan2(y, x);
	};

	float beginAngle = toAngle(begin);
	float endAngle = toAngle(end);

	float delta = endAngle - beginAngle;
	if (delta > 3.14159f) { delta -= 2.0f * 3.14159f; }
	if (delta < -3.14159f) { delta += 2.0f * 3.14159f; }

	// 円弧を分割して点を生成
	if (orient) {
		for (int i = 0; i <= pointNum; i++) {
			float t = i / (float)pointNum;
			float angle = beginAngle + t * delta;
			Vector3 p = center + radius * (std::cos(angle) * u + std::sin(angle) * v);
			points.push_back(p);
		}
	} else {
		for (int i = pointNum; i >= 0; i--) {
			float t = i / (float)pointNum;
			float angle = beginAngle + t * delta;
			Vector3 p = center + radius * (std::cos(angle) * u + std::sin(angle) * v);
			points.push_back(p);
		}
	}

	return Polyline(std::move(points));
}
void Circle::Build(float radius, int pointNum, const Vector3& center)
{
	for (int i = 0; i < pointNum; i++) {
		auto angle0 = (i / (float)pointNum) * 3.14159f * 2.0f;
		auto angle1 = ((i + 1) / (float)pointNum) * 3.14159f * 2.0f;
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
}
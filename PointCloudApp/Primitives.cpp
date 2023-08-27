#include "Primitives.h"
Cube::Cube(const vec3& min, const vec3& max)
	:m_min(min), m_max(max)
{
	m_position.resize(8);
	m_position[0] = (m_min);
	m_position[1] = (vec3(m_max.x, m_min.y, m_min.z));
	m_position[2] = (vec3(m_max.x, m_max.y, m_min.z));
	m_position[3] = (vec3(m_min.x, m_max.y, m_min.z));

	m_position[4] = (vec3(m_min.x, m_min.y, m_max.z));
	m_position[5] = (vec3(m_max.x, m_min.y, m_max.z));
	m_position[6] = (m_max);
	m_position[7] = (vec3(m_min.x, m_max.y, m_max.z));

	m_index.resize(36);
	AddIndex(0, 0, 3, 2, 1);
	AddIndex(1, 0, 4, 7, 3);
	AddIndex(2, 4, 5, 6, 7);
	AddIndex(3, 1, 2, 6, 5);
	AddIndex(4, 2, 3, 7, 6);
	AddIndex(5, 1, 5, 4, 0);


	CalcNormal();
	m_drawType = GL_TRIANGLES;
}

void Cube::AddIndex(int index, int vertex0, int vertex1, int vertex2, int vertex3)
{
	index *= 6;
	m_index[index + 0] = vertex0; m_index[index + 1] = vertex1; m_index[index + 2] = vertex2;
	m_index[index + 3] = vertex0; m_index[index + 4] = vertex2; m_index[index + 5] = vertex3;
}

Cone::Cone(float _radius, float _height, int _partition)
	: radius(_radius)
	, height(_height)
	, partition(_partition)
{
	m_position.push_back(vec3(0, height, 0));
	m_position.push_back(vec3(0, 0, 0));

	float rad = 2 * pi<float>() / partition;
	float theta = 0;

	float y = pi<float>() / 2;
	for (int i = 0; i <= partition; i++)
	{
		float x = -(i * rad);
		m_position.push_back(vec3(
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

		m_index.push_back(0);			// ��[�_
		m_index.push_back(index1);
		m_index.push_back(index2);

		// �I�[������
		m_index.push_back(1);			// ��ʂ̒��S�_
		m_index.push_back(index2);
		m_index.push_back(index1);
	}

	CalcNormal();
	m_drawType = GL_TRIANGLES;
}

Cylinder::Cylinder(float _baseRad, float _topRad, float _height, int _slices)
	: baseRad(_baseRad)
	, topRad(_topRad)
	, height(_height)
	, slices(_slices)
{
	m_position.push_back(vec3(0, 0, 0));
	m_position.push_back(vec3(0, height, 0));

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
		m_position.push_back(vec3(xPos, yPos, zPos));

		xPos = cosAngle * topRad;
		yPos = height;
		zPos = sinAngle * topRad;
		m_position.push_back(vec3(xPos, yPos, zPos));
	}

	// ��ʒ��_ => ��ʒ��_�̏��ō쐬
	// k = 0 < ��ʁE��ʕ�;
	int k = 2;
	for (int i = 0; i < slices - 1; i++)
	{
		// ���
		m_index.push_back(0);
		m_index.push_back(k);
		m_index.push_back(k + 2);

		// ���
		m_index.push_back(1);
		m_index.push_back(k + 3);
		m_index.push_back(k + 1);

		//// ����
		m_index.push_back(k);
		m_index.push_back(k + 1);
		m_index.push_back(k + 2);

		m_index.push_back(k + 2);
		m_index.push_back(k + 1);
		m_index.push_back(k + 3);
		k += 2;
	}

	// ���
	m_index.push_back(0);
	m_index.push_back(k);
	m_index.push_back(2);

	// ���
	m_index.push_back(1);
	m_index.push_back(3);
	m_index.push_back(k + 1);

	//// ����
	m_index.push_back(k);
	m_index.push_back(k + 1);
	m_index.push_back(2);

	m_index.push_back(k + 1);
	m_index.push_back(3);
	m_index.push_back(2);

	CalcNormal();

	m_drawType = GL_TRIANGLES;
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
			m_position.push_back(vec3(
				xy * cosf(sectorAngle),
				xy * sinf(sectorAngle),
				radius * sinf(stackAngle)));

			m_normal.push_back(vec3(
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

	m_drawType = GL_TRIANGLES;
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
			m_position.push_back(vec3(tx, ty, tz));

			float rx = rr * (float)cos(tr);
			float rz = rr * (float)sin(tr);
			m_normal.push_back(vec3(rx, ry, rz));

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

	m_drawType = GL_TRIANGLES;
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
	m_position.push_back(glm::vec3(-0.5, -0.5, 0.0));
	m_position.push_back(glm::vec3(0.5, -0.5, 0.0));
	m_position.push_back(glm::vec3(0.25, 0.5, 0.0));
	m_normal.push_back(glm::vec3(0.0, 0.0, 1.0));
	m_normal.push_back(glm::vec3(0.0, 0.0, 1.0));
	m_normal.push_back(glm::vec3(0.0, 0.0, 1.0));
	m_index.push_back(0);
	m_index.push_back(1);
	m_index.push_back(2);
	m_drawType = GL_TRIANGLES;
}
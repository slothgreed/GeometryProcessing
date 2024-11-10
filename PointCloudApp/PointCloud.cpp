#include "PointCloud.h"
#include "FileUtility.h"
#include "Random.h"
#include "Utility.h"
#include <iostream>
namespace KI
{

PointCloud::PointCloud()
{
}

PointCloud::~PointCloud()
{
}

void  PointCloud::To2D()
{
	for (int i = 0; i < m_position.size(); i++) {
		m_position[i].z = 0.0f;
	}
}

Vector<Vector3> PointCloud::Create3D(int size, const Vector3& min, const Vector3& max)
{
	Vector<Vector3> pos(size);
	for (size_t i = 0; i < pos.size(); i++) {
		pos[i] = Random::Vec3(min, max);
	}

	Vector<Vector3> unique;
	for (size_t i = 0; i < pos.size(); i++) {
		bool isUnique = true;
		for (size_t j = 0; j < pos.size(); j++) {
			if (i == j)continue;
			if (pos[i] == pos[j]) {
				isUnique = false;
				break;
			}
		}
		if (isUnique) {
			unique.push_back(pos[i]);
		}
	}
	return unique;
}

void PointCloud::SetColorSingle(const Vector3& color)
{
	m_color.resize(m_position.size());
	for (size_t i = 0; i < m_color.size(); i++) {
		m_color[i] = Vector4(color.x, color.y, color.z, 1.0);
	}
}

Vector<Vector4> PointCloud::CreatePosition4f()
{
	Vector<Vector4> position(m_position.size());
	for (int i = 0; i < m_position.size(); i++) {
		position[i].x = m_position[i].x;
		position[i].y = m_position[i].y;
		position[i].z = m_position[i].z;
		position[i].w = 1.0f;
	}

	return position;
}
Vector<Vector4> PointCloud::CreateColor4f()
{
	Vector<Vector4> color(m_color.size());
	for (int i = 0; i < m_color.size(); i++) {
		color[i].x = m_color[i].x;
		color[i].y = m_color[i].y;
		color[i].z = m_color[i].z;
		color[i].w = 1.0f;
	}

	return color;
}

Vector<Vector4> PointCloud::CreatePositionColor4f()
{
	Vector<Vector4> posColor(m_position.size());
	
	for (int i = 0; i < m_position.size(); i++) {
		posColor[i].x = m_position[i].x;
		posColor[i].y = m_position[i].y;
		posColor[i].z = m_position[i].z;
		if (m_position.size() == m_color.size()) {
			posColor[i].w = ColorUtility::PackColor3f(m_color[i]);
		}
	}

	return posColor;
}
}
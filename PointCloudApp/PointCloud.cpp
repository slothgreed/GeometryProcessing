#include "PointCloud.h"
#include "FileUtility.h"
#include "Random.h"
#include <iostream>
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


Vector<glm::vec4> PointCloud::CreatePosition4f()
{
	Vector<glm::vec4> position(m_position.size());
	for (int i = 0; i < m_position.size(); i++) {
		position[i].x = m_position[i].x;
		position[i].y = m_position[i].y;
		position[i].z = m_position[i].z;
		position[i].w = 1.0f;
	}

	return position;
}
Vector<glm::vec4> PointCloud::CreateColor4f()
{
	Vector<glm::vec4> color(m_color.size());
	for (int i = 0; i < m_color.size(); i++) {
		color[i].x = m_color[i].x;
		color[i].y = m_color[i].y;
		color[i].z = m_color[i].z;
		color[i].w = 1.0f;
	}

	return color;
}
#include "DelaunayGenerator.h"

namespace KI
{


DelaunayGenerator::DelaunayGenerator()
{
}

DelaunayGenerator::~DelaunayGenerator()
{

}

Vector<unsigned int> DelaunayGenerator::Execute2D(const Vector<Vector3>& points)
{
	auto hugeTriangle = CreateHugeTriangle2D(points);
	return Vector<unsigned int>();
}

DelaunayGenerator::Triangle DelaunayGenerator::CreateHugeTriangle2D(const Vector<Vector3>& points)
{
	Vector3 center = Vector3(0);
	for (const auto& p : points) {
		center += p;
	}

	center /= points.size();

	float maxLength = 0;
	Vector3 maxPos;
	for (const auto& p : points) {
		auto len = glm::length2(p - center);
		if (maxLength < len) {
			maxLength = len;
			maxPos = p;
		}
	}

	// ‰~‚ð•¢‚¤³ŽOŠpŒ`‚ð\’z
	Triangle tri;
	Vector3 radDir = (center - maxPos);
	float radius = glm::length(radDir);
	float root3 = sqrtf(radius);
	tri.pos0 = center + radDir + radDir;
	tri.pos1 = maxPos + (root3 * Vector3(radDir.x, -radDir.y, 0.0));
	tri.pos2 = maxPos - (root3 * Vector3(radDir.x, -radDir.y, 0.0));

	return tri;
}


void DelaunayGenerator::ShowUI()
{

}
}
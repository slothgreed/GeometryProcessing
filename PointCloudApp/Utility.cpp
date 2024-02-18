#include "Utility.h"
#include "Random.h"
#include "FileUtility.h"
namespace KI
{
Vector<Vector3> g_RGB;

void Printf::Vec3(const String& name, const Vector3& value, bool newLine)
{
	if (newLine) {
		printf((name + ":%lf,%lf,%lf\n").data(), value.x, value.y, value.z);
	} else {
		printf((name + ":%lf,%lf,%lf").data(), value.x, value.y, value.z);
	}
}
Vector3 Random::Vec3(float min, float max)
{
	Vector3 value;
	value.r = Gaccho::rnd(min, max);
	value.g = Gaccho::rnd(min, max);
	value.b = Gaccho::rnd(min, max);
	return value;
}

int Random::Int(float min, float max)
{
	return Gaccho::rnd(min, max);
}
Vector3 Random::Vec3(const Vector3& min, const Vector3& max)
{
	Vector3 value;
	value.r = Gaccho::rnd(min[0], max[0]);
	value.g = Gaccho::rnd(min[1], max[1]);
	value.b = Gaccho::rnd(min[2], max[2]);
	return value;
}
Vector3 ColorUtility::CreateRandom()
{
	Vector3 color;
	color.r = Gaccho::rnd(0, 255) / 255.0f;
	color.g = Gaccho::rnd(0, 255) / 255.0f;
	color.b = Gaccho::rnd(0, 255) / 255.0f;
	return color;
}
Vector3 ColorUtility::CreatePrimary(int index)
{
	if (index > 7) { index %= 7; }
	if (index == 0) {
		return Vector3(1, 0, 0);
	} else if (index == 1) {
		return Vector3(0, 1, 0);
	} else if (index == 2) {
		return Vector3(0, 0, 1);
	} else if (index == 3) {
		return Vector3(1, 1, 0);
	} else if (index == 4) {
		return Vector3(1, 0, 1);
	} else if (index == 5) {
		return Vector3(0, 1, 1);
	} else if (index == 6) {
		return Vector3(1, 1, 1);
	} else if (index == 7) {
		return Vector3(0, 0, 0);
	}

	return Vector3(0, 0, 0);
}
Vector3 ColorUtility::CreatePseudo(float value, float minValue, float maxValue)
{
	if (g_RGB.size() == 0) { InitializePseudoColor(); }

	if (maxValue < value) {
		return g_RGB[255];
	}
	if (0 > value || (maxValue - minValue) == 0) {
		return g_RGB[0];
	}

	float scale = 255 * (value - minValue) / (maxValue - minValue);
	return g_RGB[(int)scale];
}
Vector3 ColorUtility::CreatePseudo(unsigned int value, unsigned int maxValue)
{
	return CreatePseudo(value, 0, maxValue);
}

void ColorUtility::InitializePseudoColor()
{
	g_RGB.resize(256);
	float scale = 4;
	for (int i = 0; i < 256; i++) {
		if (i <= 63) {
			g_RGB[i].x = 0 / 255.0f;
			g_RGB[i].y = scale * i / 255.0f;
			g_RGB[i].z = 255 / 255.0f;
			continue;
		}
		if (i <= 127) {
			g_RGB[i].x = 0 / 255.0f;
			g_RGB[i].y = 255 / 255.0f;
			g_RGB[i].z = (255 - (scale * (i - 64))) / 255.0f;
			continue;
		}
		if (i <= 191) {
			g_RGB[i].x = (scale * (i - 127)) / 255.0f;
			g_RGB[i].y = 255 / 255.0f;
			g_RGB[i].z = 0 / 255.0f;
			continue;
		}
		if (i <= 255) {
			g_RGB[i].x = 255 / 255.0f;
			g_RGB[i].y = (255 - (scale * (i - 192))) / 255.0f;
			g_RGB[i].z = 0 / 255.0f;
			continue;
		}
	}
}
}
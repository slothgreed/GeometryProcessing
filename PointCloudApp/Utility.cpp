#include "Utility.h"
#include "Random.h"
#include "FileUtility.h"
std::vector<vec3> g_RGB;
vec3 ColorUtility::CreateRandom()
{
	vec3 color;
	color.r = Gaccho::rnd(0, 255) / 255.0f;
	color.g = Gaccho::rnd(0, 255) / 255.0f;
	color.b = Gaccho::rnd(0, 255) / 255.0f;
	return color;
}
vec3 ColorUtility::CreatePseudo(unsigned int value, unsigned int maxValue)
{
	if (g_RGB.size() == 0) { InitializePseudoColor(); }
	
	if (maxValue < value) {
		return g_RGB[255];
	}
	if (0 > value) {
		return g_RGB[0];
	}

	float scale = 255 * (value) / maxValue;
	return g_RGB[(int)scale];
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

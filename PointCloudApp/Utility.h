#ifndef UTILITY_H
#define UTILITY_H

class Random
{
public:
	Random() {};
	~Random() {};

	static int Int(float min, float max);
	static vec3 Vec3(float min, float max);
	static vec3 Vec3(const vec3& min, const vec3& max);

private:

};

class Printf
{
public:
	Printf() {};
	~Printf() {};

	static void Vec3(const string& name, const vec3& value, bool newLine = true);

private:

};


class ColorUtility
{
public:
	ColorUtility() {};
	~ColorUtility() {};

	static vec3 CreateRandom();
	static vec3 CreatePrimary(int index); // å¥êF
	static vec3 CreatePseudo(unsigned int value, unsigned int maxValue);
private:
	static void InitializePseudoColor();
};

#endif UTILITY_H

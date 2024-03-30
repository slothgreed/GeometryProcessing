#ifndef UTILITY_H
#define UTILITY_H
namespace KI
{
class Random
{
public:
	Random() {};
	~Random() {};

	static int Int(float min, float max);
	static Vector3 Vec3(float min, float max);
	static Vector3 Vec3(const Vector3& min, const Vector3& max);

private:

};

class Printf
{
public:
	Printf() {};
	~Printf() {};

	static void Vec3(const String& name, const Vector3& value, bool newLine = true);

private:

};


class ColorUtility
{
public:
	ColorUtility() {};
	~ColorUtility() {};

	static Vector3 CreateRandom();
	static Vector3 CreatePrimary(int index); // å¥êF
	static Vector3 CreatePseudo(unsigned int value, unsigned int maxValue);
	static Vector3 CreatePseudo(float value, float minValue, float maxValue);
	static unsigned int PackColor3f(const Vector3& value);
	static Vector3 UnPackColor3f(unsigned int value);
private:
	static void InitializePseudoColor();
};
}
#endif UTILITY_H

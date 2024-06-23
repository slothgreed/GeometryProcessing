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
	static void Vec4(const String& name, const Vector4& value, bool newLine = true);
	static void Mat4(const String& name, const Matrix4x4& value, bool newLine = true);

private:

};


class ColorUtility
{
public:
	ColorUtility() {};
	~ColorUtility() {};

	static Vector3 CreateRandom();
	static Vector4 CreateRandom4();
	static Vector3 CreatePrimary(int index); // ���F
	static Vector4 CreatePrimary4(int index);
	static Vector3 CreatePseudo(unsigned int value, unsigned int maxValue);
	static Vector3 CreatePseudo(float value, float minValue, float maxValue);
	static unsigned int PackColor3f(const Vector3& value);
	static Vector3 UnPackColor3f(unsigned int value);
private:
	static void InitializePseudoColor();
};

class TypeConverter
{
public:
	TypeConverter();
	~TypeConverter();

	static Vector<Vector4> Convert4f(const Vector<Vector3>& data);
private:

};

}
#endif UTILITY_H

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
	static Vector3 CreatePrimary(int index); // å¥êF
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


namespace glmUtil
{
	const float eps = 0.0001f;
	inline bool iszero(const Vector3& value)
	{
		return
			-eps < value.x && value.x < eps &&
			-eps < value.y && value.y < eps &&
			-eps < value.z && value.z < eps;
	}

	inline Matrix4x4 CreateScale(const Vector3& value)
	{
		return glm::scale(mat4(1), value);
	}

	inline Matrix4x4 CreateScale(float value)
	{
		return glm::scale(mat4(1), Vector3(value, value, value));
	}

	inline Matrix4x4 CreateRotate(float rad, const Vector3& axis)
	{
		return glm::rotate(glm::mat4(1), rad, axis);
	}

}
}
#endif UTILITY_H

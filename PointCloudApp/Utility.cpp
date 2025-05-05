#include "Utility.h"
#include "Random.h"
#include "FileUtility.h"
namespace KI
{
Vector<Vector3> g_RGB;

void Printf::Vec3(const Vector3& value, bool newLine)
{
	printf(("% lf, % lf, % lf\n"), value.x, value.y, value.z);
}
void Printf::Vec3(const String& name, const Vector3& value, bool newLine)
{
	if (newLine) {
		printf((name + ":%lf,%lf,%lf\n").data(), value.x, value.y, value.z);
	} else {
		printf((name + ":%lf,%lf,%lf").data(), value.x, value.y, value.z);
	}
}

void Printf::Vec4(const String& name, const Vector4& value, bool newLine)
{
	if (newLine) {
		printf((name + ":%lf,%lf,%lf,%lf\n").data(), value.x, value.y, value.z, value.w);
	} else {
		printf((name + ":%lf,%lf,%lf,%lf").data(), value.x, value.y, value.z, value.w);
	}
}

void Printf::Mat4(const String& name, const Matrix4x4& mat, bool newLine)
{
	printf(
		"%.3lf,%.3lf,%.3lf,%.3lf\n%.3lf,%.3lf,%.3lf,%.3lf\n%.3lf,%.3lf,%.3lf,%.3lf\n%.3lf,%.3lf,%.3lf,%.3lf\n",
		mat[0][0], mat[0][1], mat[0][2], mat[0][3],
		mat[1][0], mat[1][1], mat[1][2], mat[1][3],
		mat[2][0], mat[2][1], mat[2][2], mat[2][3],
		mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
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

float Random::Float(float min, float max)
{
	return Gaccho::rnd(min, max);
}
Vector2 Random::Vec2(const Vector2& min, const Vector2& max)
{
	Vector2 value;
	value.x = Gaccho::rnd(min.x, max.x);
	value.y = Gaccho::rnd(min.y, max.y);
	return value;
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

Vector4 ColorUtility::CreateRandom4()
{
	Vector4 color;
	color.r = Gaccho::rnd(0, 255) / 255.0f;
	color.g = Gaccho::rnd(0, 255) / 255.0f;
	color.b = Gaccho::rnd(0, 255) / 255.0f;
	color.a = 1.0f;
	return color;
}
Vector4 ColorUtility::CreatePrimary4(int index)
{
	return Vector4(CreatePrimary(index), 1.0);
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

unsigned int ColorUtility::PackColor3f(const Vector3& value)
{
	unsigned int color = 0;
	color |= ((int)(value.x * 255) & 255) << 16;
	color |= ((int)(value.y * 255) & 255) << 8;
	color |= ((int)(value.z * 255) & 255);
	return color;
}
Vector3 ColorUtility::UnPackColor3f(unsigned int value)
{
	Vector3 color;
	color.x = (value >> 16) & 255 / 255;
	color.y = (value >> 8) & 255 / 255;
	color.z = (value) & 255 / 255;
	return color;
}


Vector<Vector4> TypeConverter::Convert4f(const Vector<Vector3>& data)
{
	Vector<Vector4> ret;
	ret.resize(data.size());
	for (size_t i = 0; i < ret.size(); i++) {
		ret[i].x = data[i].x;
		ret[i].y = data[i].y;
		ret[i].z = data[i].z;
		ret[i].w = 1.0f;
	}

	return ret;
}

UIntBool::UIntBool(int size)
{
	m_data.resize(UIntBool::Size(size));
}

bool UIntBool::True(int pos) const
{
	auto index = pos / size_t(32);
	auto bit = size_t(31) - (pos % 32);
	auto mask = 1 << bit;
	return m_data[index] & mask;
}

int UIntBool::Size(int num)
{
	return (num + 32 - 1) / 32;
}

int UIntBool::MemorySize() const
{
	return m_data.size() * sizeof(unsigned int);
}
bool glmUtil::iszero(const Vector3& value)
{
	return
		-eps < value.x && value.x < eps &&
		-eps < value.y && value.y < eps &&
		-eps < value.z && value.z < eps;
}

Matrix4x4 glmUtil::CreateScale(const Vector3& value)
{
	return glm::scale(mat4(1), value);
}

Matrix4x4 glmUtil::CreateScale(float value)
{
	return glm::scale(mat4(1), Vector3(value, value, value));
}

Matrix4x4 glmUtil::CreateRotate(float rad, const Vector3& axis)
{
	return glm::rotate(glm::mat4(1), rad, axis);
}

Matrix4x4 glmUtil::CreateRotate(const Vector3& angle)
{
	Matrix4x4 rotX = glm::rotate(Matrix4x4(1.0f), angle.x, Vector3(1.0f, 0.0f, 0.0f));
	Matrix4x4 rotY = glm::rotate(Matrix4x4(1.0f), angle.y, Vector3(0.0f, 1.0f, 0.0f));
	Matrix4x4 rotZ = glm::rotate(Matrix4x4(1.0f), angle.z, Vector3(0.0f, 0.0f, 1.0f));

	//return rotZ * rotY * rotX;

	return rotZ * rotX * rotY;
}

Matrix4x4 glmUtil::CreateTranslate(const Vector3& translate)
{
	return glm::translate(Matrix4x4(1.0f), translate);
}

Matrix4x4 glmUtil::CreateTransform(float scale, const Vector3& translate)
{
	return glm::scale(glm::translate(Matrix4x4(1.0f), translate), Vector3(scale, scale, scale));
}

Matrix4x4 glmUtil::CreateRotate(Vector3 from, Vector3 to)
{
	// “ü—ÍƒxƒNƒgƒ‹‚ð³‹K‰»
	from = glm::normalize(from);
	to = glm::normalize(to);

	// “àÏ‚ÆŠOÏ‚ðŒvŽZ
	float dot = glm::clamp(glm::dot(from, to), -1.0f, 1.0f);
	auto axis = glm::cross(from, to);

	// Šp“x‚ðŽZo
	float angle = acos(dot);

	// ‰ñ“]Ž²‚ÆŠp“x‚©‚ç‰ñ“]s—ñ‚ðì¬
	return glm::rotate(glm::mat4(1.0f), angle, axis);
}
Vector3 glmUtil::ToScale(const Matrix4x4& matrix)
{
	Matrix3x3 mat(matrix);

	Vector3 scale;
	scale.x = glm::length(mat[0]);
	scale.y = glm::length(mat[1]);
	scale.z = glm::length(mat[2]);

	return scale;
}

Vector3 glmUtil::ToRotateAngle(const Matrix4x4& matrix)
{
	Matrix3x3 rotation(matrix);

	Vector3 scale = ToScale(matrix);

	rotation[0] /= scale.x;
	rotation[1] /= scale.y;
	rotation[2] /= scale.z;

	Vector3 eulerAngles;

	// ŠeŽ²‚Ì‰ñ“]‚ðŽæ“¾ (YXZ‡)
	if (rotation[2][0] < 1) {
		if (rotation[2][0] > -1) {
			eulerAngles.y = std::asin(-rotation[2][0]);
			eulerAngles.x = std::atan2(rotation[2][1], rotation[2][2]);
			eulerAngles.z = std::atan2(rotation[1][0], rotation[0][0]);
		} else {
			// rotation[2][0] == -1
			eulerAngles.y = glm::pi<float>() / 2.0f;
			eulerAngles.x = std::atan2(-rotation[0][1], rotation[1][1]);
			eulerAngles.z = 0.0f;
		}
	} else {
		// rotation[2][0] == 1
		eulerAngles.y = -glm::pi<float>() / 2.0f;
		eulerAngles.x = std::atan2(-rotation[0][1], rotation[1][1]);
		eulerAngles.z = 0.0f;
	}
	return eulerAngles;
}


String glmUtil::ToString(const Vector2& value)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(6);
	ss << "(" << value.x << ", " << value.y << ")";
	return ss.str();
}

String glmUtil::ToString(const Vector3& value)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(6); 
	ss << "(" << value.x << ", " << value.y << ", " << value.z << ")";
	return ss.str();
}

String glmUtil::ToString(const Vector4& value)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(6); 
	ss << "(" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << ")";
	return ss.str();
}

String glmUtil::ToString(const Matrix4x4& mat)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(6);
	ss << "(" << mat[0][0] << ", " << mat[0][1] << ", " << mat[0][2] << ", " << mat[0][3] << ")";
	ss << "(" << mat[1][0] << ", " << mat[1][1] << ", " << mat[1][2] << ", " << mat[1][3] << ")";
	ss << "(" << mat[2][0] << ", " << mat[2][1] << ", " << mat[2][2] << ", " << mat[2][3] << ")";
	ss << "(" << mat[3][0] << ", " << mat[3][1] << ", " << mat[3][2] << ", " << mat[3][3] << ")";
	return ss.str();


}

Vector3 glmUtil::ToTranslate(const Matrix4x4& matrix)
{
	return matrix[3];
}

Quaternion glmUtil::CreateQuart(const Vector4& vec)
{
	Quaternion quart;
	quart.x = vec.x;
	quart.y = vec.y;
	quart.z = vec.z;
	quart.w = vec.w;
	return quart;
}

int GLUtil::GetPrimitiveSize(int value)
{
	if (GL_POINTS == value) { return 1; }
	if (GL_LINES == value) { return 2; }
	if (GL_TRIANGLES == value) { return 3; }

	return 0;
}


}
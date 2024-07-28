#ifndef GL_UTILITY_H
#define GL_UTILITY_H
#include "pch.h"
namespace KI
{
inline Quaternion CreateQuart(const Vector4& vec)
{
	Quaternion quart;
	quart.x = vec.x;
	quart.y = vec.y;
	quart.z = vec.z;
	quart.w = vec.w;
	return quart;
}

inline Matrix4x4 CreateRotateMatrix(const Vector4& vec)
{
	return glm::mat4_cast(CreateQuart(vec));
}

inline Matrix4x4 CreateScale(float scale)
{
	return glm::scale(glm::mat4x4(1.0), vec3(scale, scale, scale));
}

}


#endif GL_UTILITY_H
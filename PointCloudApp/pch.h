#ifndef PCH_H
#define PCH_H
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <GL/GL.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <String>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>

using namespace glm;

namespace KI
{

enum MY_MOUSE_BUTTON : unsigned int
{
	MOUSE_BUTTON_LEFT = 1,
	MOUSE_BUTTON_RIGHT = 2,
	MOUSE_BUTTON_MIDDLE = 4
};


enum MY_MOUSE_EVENT
{
	MOUSE_EVENT_WHEEL,
	MOUSE_EVENT_DOWN,
	MOUSE_EVENT_UP,
	MOUSE_EVENT_MOVE
};

template <typename T> using Shared= std::shared_ptr<T>;
template <typename T> using Unique = std::unique_ptr<T>;
template <typename T> using Vector = std::vector<T>;
template <typename T> using Optional = std::optional<T>;

using String = std::string;
using Matrix4x4 = glm::mat4x4;
using Vector2 = glm::vec2;
using Vector3 = glm::vec3;
using Vector4 = glm::vec4;
using Quaternion = glm::quat;

inline int StringToInt(const String& str)
{
	return std::atoi(str.data());
}

inline String IntToString(int value)
{
	return std::to_string(value);
}




void getError();

#define OUTPUT_GLERROR getError();

}

#endif  PCH_H

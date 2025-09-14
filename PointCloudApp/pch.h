#ifndef PCH_H
#define PCH_H
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <GL/GL.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "GLAPIExt.h"
#include <iostream>
#include <algorithm>
#include <String>
#include <queue>
#include <vector>
#include <list>
#include <memory>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stack>
#include <bitset>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include "imgui.h"
#define EIGEN_DONT_PARALLELIZE
#include <Eigen/Core>
#include <Eigen/SVD>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#define NOMINMAX
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
template <typename T> using List = std::list<T>;
template <typename T> using Shared= std::shared_ptr<T>;
template <typename T> using Unique = std::unique_ptr<T>;
template <typename T> using Vector = std::vector<T>;
template <typename T> using Optional = std::optional<T>;
template <typename T> using USet = std::unordered_set<T>;
template <std::size_t T> using BitSet = std::bitset<T>;
using String = std::string;
using Matrix3x3 = glm::mat3x3;
using Matrix4x4 = glm::mat4x4;
using Vector2 = glm::vec2;
using Vector2i = glm::ivec2;
using Vector3 = glm::vec3;
using Vector3i = glm::ivec3;
using Vector3u = glm::uvec3;
using Vector4 = glm::vec4;
using Vector4i = glm::ivec4;
using Vector4u = glm::uvec4;
using Quaternion = glm::quat;

inline int StringToInt(const String& str)
{
	return std::atoi(str.data());
}

inline String IntToString(int value)
{
	return std::to_string(value);
}

inline int CeilDiv(int a, int b)
{
	return (a + b - 1) / b;
}



void getError();

#define OUTPUT_GLERROR getError();
#define RELEASE_INSTANCE(x) if(x) { delete x; x = nullptr;}
#define RELEASE_VECTOR(x) for(size_t i = 0; i < x.size(); i++) if(x[i]) { delete x[i];} x.clear();

}

#endif  PCH_H

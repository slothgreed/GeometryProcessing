#ifndef PCH_H
#define PCH_H
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <GL/GL.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

using namespace std;
using namespace glm;



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


void getError();

#define OUTPUT_GLERROR getError();

#endif  PCH_H

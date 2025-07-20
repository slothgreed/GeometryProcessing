struct Camera
{
	mat4x4 view;
	mat4x4 proj;
	mat4x4 VP;
	vec4 eye;
	vec4 center;
	vec2 viewSize;
	float padding[6];
};

struct Light
{
	vec4 color;
	vec4 direction;
	float padding[56];
};


vec4 tonemap(vec4 color)
{
	vec4 x = max(vec4(0),color * 16 - 0.004);
	return (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
}

vec4 IntToVec4(int value)
{
	float r = float((value >> 24) & 0xFF) / 255.0;
    float g = float((value >> 16) & 0xFF) / 255.0;
    float b = float((value >> 8) & 0xFF) / 255.0;
    float a = float(value & 0xFF) / 255.0;
    return vec4(r, g, b, a);
}

float packColor(vec3 color) {
    return color.r + color.g * 256.0 + color.b * 256.0 * 256.0;
}

vec4 unpackColor(float f) {
    float fi = round(f);
    vec3 color;
    color.r = floor(fi / 65536.0); // 256 * 256
    fi -= color.r * 65536.0;
    color.g = floor(fi / 256.0);
    fi -= color.g * 256.0;
    color.b = fi;
    return vec4(color / 255.0, 1.0);
}
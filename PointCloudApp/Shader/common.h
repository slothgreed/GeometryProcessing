struct Camera
{
	mat4x4 VP;
	vec4 eye;
	float padding[44];
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
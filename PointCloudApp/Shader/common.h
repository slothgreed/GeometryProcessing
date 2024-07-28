struct Camera
{
	mat4x4 VP;
	float padding[48];
};

vec4 tonemap(vec4 color)
{
	vec4 x = max(vec4(0),color * 16 - 0.004);
	return (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
}
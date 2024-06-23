#version 450
layout (location = 0) in vec3 position;

uniform mat4 u_VP;
uniform mat4 u_Model;
uniform vec3 u_Color;
out vec4 f_color;
void main()
{
    gl_Position = u_VP * u_Model * vec4(position.x, position.y, position.z, 1.0);
	f_color = vec4(u_Color,1.0);
}
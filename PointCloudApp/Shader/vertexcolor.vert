#version 400
layout (location = 0) in vec3 position;
layout (location = 2) in vec3 color;

uniform mat4 u_VP;
uniform mat4 u_Model;

out vec4 f_color;
void main()
{
    gl_Position = u_VP * u_Model * vec4(position.x, position.y, position.z, 1.0);
	f_color = vec4(color,1.0);
}
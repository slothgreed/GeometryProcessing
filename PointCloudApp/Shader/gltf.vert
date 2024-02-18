#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec4 tangent;

uniform mat4 u_VP;
uniform mat4 u_Model;

out vec3 f_normal;
out vec2 f_texcoord;
out vec4 f_tangent;
void main()
{
    gl_Position = u_VP * u_Model * vec4(position.x, position.y, position.z, 1.0);
	f_normal = normal;
	f_texcoord = texcoord;
	f_tangent = tangent;
}
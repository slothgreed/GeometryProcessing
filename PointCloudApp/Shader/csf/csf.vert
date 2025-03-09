layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;

layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

layout(std430, binding = 3) buffer NodeBuffer
{
	CSFNode nodes[];
};

uniform ivec2 u_node; // (x,y) = (matrix,material);

out vec4 f_position;
out vec4 f_normal;

void main()
{
	f_position = nodes[u_node.x].world * position;
	f_normal = normal;
	gl_Position = camera.VP * f_position;
}
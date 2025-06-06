
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
uniform mat4 u_Model;
uniform vec3 u_Color;
out vec4 f_position;
out vec4 f_color;
out vec3 f_normal;
layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

void main()
{
	f_position = u_Model * vec4(position.x, position.y, position.z, 1.0);
	f_color = vec4(u_Color,1.0);
	f_normal = normal;
    gl_Position = camera.VP * f_position;
}
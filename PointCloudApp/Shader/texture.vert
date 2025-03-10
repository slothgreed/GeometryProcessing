layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
uniform mat4 u_Model;

out vec2 f_texcoord;

layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

void main()
{
    gl_Position = camera.VP * u_Model * vec4(position, 1.0);
	f_texcoord = texcoord;
}
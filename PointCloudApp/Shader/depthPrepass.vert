layout(location = 0) in vec3 position;

uniform mat4 u_Model;

layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

void main()
{
	gl_Position = camera.VP * u_Model * vec4(position, 1.0);
}

layout (location = 0) in vec3 position;

layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

uniform mat4 u_Model;

out vec3 texCoord;
void main()
{
    vec4 pos = camera.VP * u_Model * vec4(position.x,position.y,position.z, 1.0);
	gl_Position = pos;
	texCoord = position;
}
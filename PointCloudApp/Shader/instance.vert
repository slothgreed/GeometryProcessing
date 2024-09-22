layout (location = 0) in vec3 position;

layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

layout (binding=0) uniform samplerBuffer matrixTex;
uniform vec3 u_Color;
out vec4 f_color;
void main()
{
	int index = gl_InstanceID * 4;
	mat4 modelMat = mat4(
		texelFetch(matrixTex,index + 0),
		texelFetch(matrixTex,index + 1),
		texelFetch(matrixTex,index + 2),
		texelFetch(matrixTex,index + 3)
	);
    gl_Position = camera.VP * modelMat * vec4(position.x, position.y, position.z, 1.0);
	f_color = vec4(u_Color,1.0);
}
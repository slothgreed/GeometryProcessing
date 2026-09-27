layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

layout(std430, binding = 1) buffer PointLightBuffer
{
	PointLight pointLights[];
};

flat out uint f_lightIndex;
layout(location = 0) in vec3 position;
// Negative index draws the original center points; otherwise draw a unit sphere.
uniform int u_lightIndex = -1;

void main()
{
	f_lightIndex = uint(u_lightIndex < 0 ? gl_VertexID : u_lightIndex);
	vec4 light = pointLights[f_lightIndex].positionRadius;
	mat4 model = mat4(1.0);
	model[3] = vec4(light.xyz, 1.0);
	vec3 localPosition = vec3(0.0);
	if (u_lightIndex >= 0) {
		model[0][0] = light.w;
		model[1][1] = light.w;
		model[2][2] = light.w;
		localPosition = position;
	}
	gl_Position = camera.VP * model * vec4(localPosition, 1.0);
}

layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

layout(std430, binding = 1) buffer PointLightBuffer
{
	PointLight pointLights[];
};

flat out uint f_lightIndex;

void main()
{
	f_lightIndex = uint(gl_VertexID);
	gl_Position = camera.VP * vec4(pointLights[gl_VertexID].positionRadius.xyz, 1.0);
}

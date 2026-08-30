layout(location = 0) out vec4 FragColor;

layout(std430, binding = 1) buffer PointLightBuffer
{
	PointLight pointLights[];
};

flat in uint f_lightIndex;

void main()
{
	FragColor = vec4(pointLights[f_lightIndex].colorIntensity.rgb, 1.0);
}

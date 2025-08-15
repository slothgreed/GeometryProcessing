
layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 PositionTexture;
layout(location = 2)out vec4 NormalTexture;

uniform samplerCube u_prefilter;
uniform samplerCube u_irradiance;
uniform sampler2D u_brdf;

in vec4 f_color;
in vec3 f_normal;
in vec4 f_position;

layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

layout(std430, binding = 1) buffer LightBuffer
{
	Light light;
};

layout(std430, binding = 2) buffer PBRReourceBuffer
{
	PBRReource pbrResource;
};

void main()
{
	PBRInfo pbrInputs;
	CalcPBRAngle(pbrInputs, camera.eye.xyz, normalize(light.direction.xyz), f_normal, f_position.xyz);
	CalcPBRMaterial(pbrInputs, f_color.xyz, 0.4, 0.0);
	vec4 resultColor = vec4(getPBRColor(pbrInputs,light.color.rgb),1.0);
	vec4 ibl = vec4(getIBLColor(pbrInputs, pbrResource, u_brdf, u_irradiance, u_prefilter),1.0);
	resultColor += ibl;
	
    FragColor = resultColor;
	PositionTexture = f_position;
	NormalTexture = vec4((f_normal.xyz + 1) * 0.5f, 1.0f);
} 
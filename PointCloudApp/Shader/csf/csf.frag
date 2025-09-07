layout(location = 0)out vec4 FragColor;
in vec4 f_position;
in vec4 f_normal;
uniform samplerCube u_prefilter;
uniform samplerCube u_irradiance;
uniform sampler2D u_brdf;
layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

layout(std430, binding = 1) buffer LightBuffer
{
	Light light;
};

layout(std430, binding = 2) buffer MaterialBuffer
{
	CSFMaterial materials[];
};

layout(std430, binding = 4) buffer PBRGlobalBuffer
{
	PBRGlobal pbrGlobal;
};

uniform ivec2 u_node; // (x,y) = (matrix,material);

void main()
{
	PBRInfo pbrInputs;
	CSFMaterial material = materials[u_node.y];
	CalcPBRAngle(pbrInputs, camera.eye.xyz, normalize(light.direction.xyz), normalize(f_normal.xyz) * (gl_FrontFacing ? 1 : -1), f_position.xyz);
	CalcPBRMaterial(pbrInputs, material.diffuse.xyz, material.roughness, material.metallic);
	vec4 resultColor = vec4(getPBRColor(pbrInputs,light.color.rgb),1.0);
	vec4 ibl = vec4(getIBLColor(pbrInputs, pbrGlobal, u_brdf, u_irradiance, u_prefilter),1.0);
	resultColor += ibl;
	
	FragColor = resultColor;
} 
layout(location = 0)out vec4 FragColor;
in vec4 f_position;
in vec4 f_normal;

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

uniform ivec2 u_node; // (x,y) = (matrix,material);

vec4 shade(CSFMaterial material)
{
	vec4 color = vec4(0);
	color += material.ambient + material.emissive;
	vec3 eyePos = camera.eye.xyz;

	vec3 lightDir = normalize(light.direction.xyz);
	vec3 viewDir = normalize(eyePos - f_position.xyz);
	vec3 halfDir = normalize(lightDir + viewDir);
	vec3 normal = normalize(f_normal.xyz) * (gl_FrontFacing ? 1 : -1);;
	
	color += material.diffuse * max(dot(normal,lightDir),0);
	color += material.specular * pow(max(0,dot(normal,halfDir)),16);
	
	return color;
}

void main()
{
	FragColor = shade(materials[u_node.y]);
} 
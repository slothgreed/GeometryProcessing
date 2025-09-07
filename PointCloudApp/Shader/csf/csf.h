struct CSFNode
{
	mat4 matrix;
	mat4 world;
	mat4 matrixIT;
	mat4 worldIT;
};

struct CSFMaterial
{
	vec4 diffuse;
	float roughness;
	float metallic;
};
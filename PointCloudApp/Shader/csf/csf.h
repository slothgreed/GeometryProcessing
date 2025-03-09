struct CSFNode
{
	mat4 matrix;
	mat4 world;
	mat4 matrixIT;
	mat4 worldIT;
};

struct CSFMaterial
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 emissive;
	float padding[192];
};
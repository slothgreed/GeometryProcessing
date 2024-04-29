#version 430
struct GLTFNode
{
	mat4 matrix;
	mat4 localMatrix;
	int jointCount;
	float padding[31];
} node;

struct GLTFMaterial
{
	vec4 baseColor;
	int baseTexture;
	float metalic;
	int roughnessTexture;
	int normalTexture;
	float normalScale;
	float alphaCuttoff;
	int alphaMode;
	int doubleSided;
	float padding[52];
};


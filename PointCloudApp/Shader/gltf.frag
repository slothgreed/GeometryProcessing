#version 430
out vec4 FragColor;
in vec2 f_texcoord;
in vec4 f_tangent;
in vec3 f_normal;

uniform sampler2D u_colorTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_roughnessTexture;
uniform ivec2 u_ssboIndex; // (x,y,z,w) = (matrix,material,Hoge,Hoge);


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
	bool doubleSided;
	float padding[9];
};

layout(std430, binding = 3) buffer Material
{
	GLTFMaterial materials[];
};


void main()
{
	GLTFMaterial material = materials[u_ssboIndex.y];
	if(material.baseTexture == -1){
		FragColor = material.baseColor;
	}else{
		FragColor = vec4(texture(u_colorTexture,f_texcoord).rgb,1.0);
	}
	//FragColor = vec4(1,0,0,1);
} 
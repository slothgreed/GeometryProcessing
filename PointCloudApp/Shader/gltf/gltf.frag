layout(location = 0)out vec4 FragColor;
in vec3 f_worldPos;
in vec2 f_texcoord;
in vec4 f_tangent;
in vec3 f_normal;

uniform sampler2D u_colorTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_metalRoughnessTexture;
uniform sampler2D u_occlusionTexture;
uniform sampler2D u_emissiveTexture;
uniform samplerCube u_prefilter;
uniform samplerCube u_irradiance;
uniform sampler2D u_brdf;

uniform ivec2 u_ssboIndex; // (x,y,z,w) = (matrix,material,Hoge,Hoge);
uniform int u_debugView;






layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

layout(std430, binding = 1) buffer LightBuffer
{
	Light light;
};

layout(std430, binding = 3) buffer Material
{
	GLTFMaterial materials[];
};

layout(std430, binding = 6) buffer PBRReourceBuffer
{
	PBRReource pbrResource;
};


vec3 getNormal(GLTFMaterial material)
{
	if(material.normalTexture == -1){
		return f_normal;
	}
	
	// Perturb normal, see http://www.thetenthplanet.de/archives/1180
	vec3 tangentNormal = texture(u_normalTexture, f_texcoord).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(f_worldPos);
	vec3 q2 = dFdy(f_worldPos);
	vec2 st1 = dFdx(f_texcoord);
	vec2 st2 = dFdy(f_texcoord);

	vec3 N = normalize(f_normal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}


vec3 getBaseColor(GLTFMaterial material)
{
	if(material.baseTexture == -1){
		return material.baseColor.rgb;
	}else{
		return texture(u_colorTexture,f_texcoord).rgb;
	}
}

vec3 getEmissive(GLTFMaterial material)
{
	if(material.emissiveTexture != -1){
		return material.emissiveColor.rgb * texture(u_emissiveTexture,f_texcoord).rgb;
	}else {
		return material.emissiveColor.rgb;
	}
}

float getOcculusion(GLTFMaterial material)
{
	if(material.occlusionTexture != -1){
		return texture(u_occlusionTexture,f_texcoord).r;
	}else{
		return 0.0f;
	}
}
void main()
{
	GLTFMaterial material = materials[u_ssboIndex.y];
	
	float metallic = 0.0;
	float perceptualRoughness = 0.0;
	
	// metallic roughness
	{
		metallic = material.metallic;
		perceptualRoughness = material.roughness;
		
		if(material.metalRoughnessTexture == -1){
			perceptualRoughness = clamp(perceptualRoughness,c_minRoughness,1.0);
			metallic = clamp(metallic,0.0,1.0);
		} else {
			vec4 mrSample = texture(u_metalRoughnessTexture,f_texcoord);
			perceptualRoughness = mrSample.g * perceptualRoughness;
			metallic = mrSample.b * metallic;
		}
	}
	
	vec3 baseColor = getBaseColor(material);
	vec3 normal = getNormal(material);
	PBRInfo pbrInputs;
	CalcPBRAngle(pbrInputs, camera.eye.xyz, normalize(light.direction.xyz), normal, f_worldPos);
	CalcPBRMaterial(pbrInputs, baseColor, perceptualRoughness, metallic);

	vec3 F = vec3(0);
	float G = 0.0;
	float D = 0.0;
	vec4 resultColor = vec4(getPBRColor(pbrInputs,light.color.rgb,F,G,D),1.0);
	vec4 ibl = vec4(getIBLColor(pbrInputs, pbrResource, u_brdf, u_irradiance, u_prefilter),1.0);
	resultColor += ibl;
	resultColor += getOcculusion(material) * resultColor;
	resultColor += vec4(getEmissive(material),0.0);
	
	if(u_debugView == 1){
		resultColor = vec4(getBaseColor(material),1.0);
	}else if(u_debugView == 2){
		resultColor = vec4(getNormal(material),1.0);
	}else if(u_debugView == 3){
		resultColor = vec4(texture(u_metalRoughnessTexture,f_texcoord).rgb,1.0);
	}else if(u_debugView == 4){
		float occ = getOcculusion(material);
		resultColor = vec4(occ,occ,occ,1.0);
	}else if(u_debugView == 5){
		resultColor = vec4(getEmissive(material),1.0);
	}else if(u_debugView == 6){
		resultColor = vec4(F,1.0);
	}else if(u_debugView == 7){
		resultColor = vec4(G,G,G,1.0);
	}else if(u_debugView == 8){
		resultColor = vec4(D,D,D,1.0);
	}else if(u_debugView == 9){
		resultColor = ibl;
	}
	
	FragColor = resultColor;
} 
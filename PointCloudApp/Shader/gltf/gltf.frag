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
const float M_PI = 3.141592653589793;
const float c_minRoughness = 0.04;
const float c_f0 = 0.04;


struct PBRReource
{
	int prefilteredMaxMip;
	float exposure;
	float pad1;
	float pad2;
};


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

struct PBRInfo
{
	float NdotL;                  // cos angle between normal and light direction
	float NdotV;                  // cos angle between normal and view direction
	float NdotH;                  // cos angle between normal and half vector
	float LdotH;                  // cos angle between light direction and half vector
	float VdotH;                  // cos angle between view direction and half vector
	float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
	float metalness;              // metallic value at the surface
	vec3 reflectance0;            // full reflectance color (normal incidence angle)
	vec3 reflectance90;           // reflectance color at grazing angle
	float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
	vec3 diffuseColor;            // color contribution from diffuse lighting
	vec3 specularColor;           // color contribution from specular lighting
};


vec3 diffuse(PBRInfo pbrInputs)
{
	return pbrInputs.diffuseColor / M_PI;
}

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

vec3 specularReflection(PBRInfo pbr)
{
	return 
		pbr.reflectance0 + 
		(pbr.reflectance90 - pbr.reflectance0) * 
		pow(clamp(1.0 - pbr.VdotH,0.0,1.0),5.0);
}

float geometricOcclusion(PBRInfo pbr)
{
	float r = pbr.alphaRoughness;
	float attenuationL = 
		2.0 * pbr.NdotL /
		(pbr.NdotL + sqrt(r * r + (1.0 - r * r) * pbr.NdotL * pbr.NdotL));
		
	float attenuationV = 
		2.0 * pbr.NdotV /
		(pbr.NdotV + sqrt(r * r + (1.0 - r * r) * pbr.NdotV * pbr.NdotV));
		
	return attenuationL * attenuationV;
}

float microfacetDistribution(PBRInfo pbr)
{
	float roughness = pbr.alphaRoughness * pbr.alphaRoughness;
	float f = (pbr.NdotH * roughness - pbr.NdotH) * pbr.NdotH + 1.0;
	return roughness / (M_PI * f * f);
}


vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec4 tonemapPBR(vec4 color)
{
	vec3 outcol = Uncharted2Tonemap(color.rgb * pbrResource.exposure);
	outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return vec4(pow(outcol, vec3(1.0f /2.2)), color.a);
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
	return vec4(pow(srgbIn.xyz,vec3(2.2)),srgbIn.w);
}

vec3 getIBLColor(PBRInfo pbrInputs, vec3 n, vec3 reflection)
{
	float lod = (pbrInputs.perceptualRoughness * pbrResource.prefilteredMaxMip);
	// retrieve a scale and bias to F0. See [1], Figure 3
	vec3 brdf = (texture(u_brdf, vec2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness))).rgb;
	vec3 diffuseLight = SRGBtoLINEAR(tonemapPBR(texture(u_irradiance, n))).rgb;

	vec3 specularLight = SRGBtoLINEAR(tonemapPBR(textureLod(u_prefilter, reflection, lod))).rgb;

	vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
	vec3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);

	return diffuse + specular;
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

	vec3 diffuseColor = baseColor * 1.0 - c_f0;
	diffuseColor *= (1.0 - metallic);
	
	float alphaRoughness = perceptualRoughness * perceptualRoughness;
	vec3 specularColor = mix(vec3(c_f0), baseColor.rgb, metallic);
	
	float reflectance = max(max(specularColor.r,specularColor.g),specularColor.b);
	
	float reflectance90 = clamp(reflectance * 25.0 , 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor;
	vec3 specularEnvironmentR90 = vec3(reflectance90);
	
	vec3 normal = getNormal(material);
	vec3 viewDir = normalize(camera.eye.xyz - f_worldPos);
	vec3 lightDir = normalize(light.direction.xyz);
	vec3 halfVec = normalize(lightDir + viewDir);
	vec3 reflectValue = normalize(reflect(-viewDir,normal));
	float NdotL = clamp(dot(normal,lightDir),0.001,1.0);
	float NdotV = clamp(abs(dot(normal,viewDir)),0.001,1.0);
	float NdotH = clamp(dot(normal,halfVec),0.0,1.0);
	float LdotH = clamp(dot(lightDir,halfVec),0.0,1.0);
	float VdotH = clamp(dot(viewDir,halfVec),0.0,1.0);
	
	PBRInfo pbrInputs = PBRInfo(
		NdotL,
		NdotV,
		NdotH,
		LdotH,
		VdotH,
		perceptualRoughness,
		metallic,
		specularEnvironmentR0,
		specularEnvironmentR90,
		alphaRoughness,
		diffuseColor,
		specularColor
	);
	
	vec3 F = specularReflection(pbrInputs);
	float G = geometricOcclusion(pbrInputs);
	float D = microfacetDistribution(pbrInputs);
	
	vec3 diffuseContrib = (1.0 - F) *diffuse(pbrInputs);
	vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	
	vec4 resultColor = vec4(NdotL * light.color.rgb * (diffuseContrib + specContrib),1.0);
	vec4 ibl = vec4(getIBLColor(pbrInputs, normal, reflectValue),1.0);
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
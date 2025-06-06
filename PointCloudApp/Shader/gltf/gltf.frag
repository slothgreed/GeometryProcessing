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
uniform ivec2 u_ssboIndex; // (x,y,z,w) = (matrix,material,Hoge,Hoge);
uniform int u_debugView;
const float M_PI = 3.141592653589793;
const float c_minRoughness = 0.04;
const float c_f0 = 0.04;

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

vec3 getNormal()
{
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

void main()
{
	vec3 baseColor = vec3(0);
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
		
		if(material.baseTexture == -1){
			baseColor = material.baseColor.rgb;
		}else{
			baseColor = texture(u_colorTexture,f_texcoord).rgb;
		}
	}
	
	vec3 diffuseColor = baseColor * 1.0 - c_f0;
	diffuseColor *= (1.0 - metallic);
	
	float alphaRoughness = perceptualRoughness * perceptualRoughness;
	vec3 specularColor = mix(vec3(c_f0), baseColor.rgb, metallic);
	
	float reflectance = max(max(specularColor.r,specularColor.g),specularColor.b);
	
	float reflectance90 = clamp(reflectance * 25.0 , 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor;
	vec3 specularEnvironmentR90 = vec3(reflectance90);
	
	vec3 normal = getNormal();
	vec3 viewDir = normalize(-camera.eye.xyz - f_worldPos);
	vec3 lightDir = normalize(light.direction.xyz);
	vec3 halfVec = normalize(lightDir + viewDir);
	vec3 reflect = -normalize(reflect(viewDir,normal));
	reflect.y *= -1.0f;
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
	
	if(material.occlusionTexture != -1){
		float ao = texture(u_occlusionTexture,f_texcoord).r;
		resultColor += ao * resultColor;
	}
	
	if(material.emissiveTexture != -1){
		vec3 emissive = material.emissiveColor.rgb * texture(u_emissiveTexture,f_texcoord).rgb;
		resultColor += vec4(emissive,1.0);
	}
	
	
	if(u_debugView == 1){
		if(material.baseTexture == -1){
			resultColor = vec4(material.baseColor.rgb,1.0);
		}else{
			resultColor = vec4(texture(u_colorTexture,f_texcoord).rgb,1.0);
		}
	}else if(u_debugView == 2){
		resultColor = vec4(texture(u_normalTexture,f_texcoord).rgb,1.0);
	}else if(u_debugView == 3){
		resultColor = vec4(texture(u_metalRoughnessTexture,f_texcoord).rgb,1.0);
	}else if(u_debugView == 4){
		resultColor = vec4(texture(u_occlusionTexture,f_texcoord).rgb,1.0);
	}else if(u_debugView == 5){
		resultColor = vec4(texture(u_emissiveTexture,f_texcoord).rgb,1.0);
	}else if(u_debugView == 6){
		resultColor = vec4(F,1.0);
	}else if(u_debugView == 7){
		resultColor = vec4(G,G,G,1.0);
	}else if(u_debugView == 8){
		resultColor = vec4(D,D,D,1.0);
	}
	
	FragColor = resultColor;
} 
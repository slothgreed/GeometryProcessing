const float M_PI = 3.141592653589793;
const float c_minRoughness = 0.04;
const float c_f0 = 0.04;

struct PBRGlobal
{
	int prefilteredMaxMip;
	float exposure;
	float pad1;
	float pad2;
};


struct PBRInfo
{
	float NdotL;                  // cos angle between normal and light direction
	float NdotV;                  // cos angle between normal and view direction
	float NdotH;                  // cos angle between normal and half vector
	float LdotH;                  // cos angle between light direction and half vector
	float VdotH;                  // cos angle between view direction and half vector
	vec3 normal;				  // normal // my parameter;
	vec3 reflect;				  // reflect // my parameter;
	float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
	float metalness;              // metallic value at the surface
	vec3 reflectance0;            // full reflectance color (normal incidence angle)
	vec3 reflectance90;           // reflectance color at grazing angle
	float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
	vec3 diffuseColor;            // color contribution from diffuse lighting
	vec3 specularColor;           // color contribution from specular lighting
};

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

vec4 tonemapPBR(PBRGlobal pbrGlobal, vec4 color)
{
	vec3 outcol = Uncharted2Tonemap(color.rgb * pbrGlobal.exposure);
	outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return vec4(pow(outcol, vec3(1.0f /2.2)), color.a);
}


vec4 SRGBtoLINEAR(vec4 srgbIn)
{
	return vec4(pow(srgbIn.xyz,vec3(2.2)),srgbIn.w);
}

vec3 getIBLColor(PBRInfo pbrInputs, PBRGlobal pbrGlobal, sampler2D brdfTex, samplerCube irradiance, samplerCube prefilter)
{
	float lod = (pbrInputs.perceptualRoughness * pbrGlobal.prefilteredMaxMip);
	// retrieve a scale and bias to F0. See [1], Figure 3
	vec3 brdf = (texture(brdfTex, vec2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness))).rgb;
	vec3 diffuseLight = SRGBtoLINEAR(tonemapPBR(pbrGlobal, texture(irradiance, pbrInputs.normal))).rgb;

	vec3 specularLight = SRGBtoLINEAR(tonemapPBR(pbrGlobal, textureLod(prefilter, pbrInputs.reflect, lod))).rgb;

	vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
	vec3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);

	return diffuse + specular;
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

vec3 getDiffuse(PBRInfo pbrInputs)
{
	return pbrInputs.diffuseColor / M_PI;
}

vec3 getPBRColor(PBRInfo pbrInputs, vec3 lightColor, inout vec3 F, inout float G, inout float D)
{
	F = specularReflection(pbrInputs);
	G = geometricOcclusion(pbrInputs);
	D = microfacetDistribution(pbrInputs);
	
	vec3 diffuseContrib = (1.0 - F) * getDiffuse(pbrInputs);
	vec3 specContrib = F * G * D / (4.0 * pbrInputs.NdotL * pbrInputs.NdotV);
	
	return pbrInputs.NdotL * lightColor * (diffuseContrib + specContrib);
}

vec3 getPBRColor(PBRInfo pbrInputs, vec3 lightColor)
{
	vec3 F = vec3(0);
	float G = 0.0;
	float D = 0.0;
	
	return getPBRColor(pbrInputs,lightColor,F,G,D);
}

void CalcPBRAngle(inout PBRInfo pbrInputs, vec3 eye, vec3 lightDir, vec3 normal, vec3 worldPos)
{
	vec3 viewDir = normalize(eye - worldPos);
	vec3 halfVec = normalize(lightDir + viewDir);
	float NdotL = clamp(dot(normal,lightDir),0.001,1.0);
	float NdotV = clamp(abs(dot(normal,viewDir)),0.001,1.0);
	float NdotH = clamp(dot(normal,halfVec),0.0,1.0);
	float LdotH = clamp(dot(lightDir,halfVec),0.0,1.0);
	float VdotH = clamp(dot(viewDir,halfVec),0.0,1.0);
	
	pbrInputs.normal = normal;
	pbrInputs.reflect = normalize(reflect(-viewDir,normal));
	pbrInputs.NdotL = NdotL;
	pbrInputs.NdotV = NdotV;
	pbrInputs.NdotH = NdotH;
	pbrInputs.LdotH = LdotH;
	pbrInputs.VdotH = VdotH;
}

void CalcPBRMaterial(inout PBRInfo pbrInputs, vec3 color, float roughness, float metallic)
{
	vec3 diffuseColor = (color * 1.0 - c_f0) * (1.0 - metallic);
	vec3 specularColor = mix(vec3(c_f0), color.rgb, metallic);
	float reflectance = max(max(specularColor.r,specularColor.g),specularColor.b);
	
	
	pbrInputs.perceptualRoughness =	roughness;
	pbrInputs.metalness = metallic;
	pbrInputs.reflectance0 = specularColor;
	pbrInputs.reflectance90 =  vec3(clamp(reflectance * 25.0 , 0.0, 1.0));
	pbrInputs.alphaRoughness = roughness * roughness;
	pbrInputs.diffuseColor = diffuseColor;
	pbrInputs.specularColor = specularColor;
}
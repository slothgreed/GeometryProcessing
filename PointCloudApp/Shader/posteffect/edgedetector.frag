layout(location = 0)out vec4 FragColor;
in vec2 f_texcoord;
layout (binding = 0) uniform sampler2D u_colorTex;
layout (binding = 1) uniform sampler2D u_normalTex;
layout (binding = 2) uniform sampler2D u_depthTex;
uniform vec2 u_texelSize; 
uniform float u_diffNormal;
uniform float u_diffDepth;
float getDepthDiff(vec2 uv, vec2 diff)
{
	float depth = -4* texture(u_depthTex,uv).r;
	float depthX1 = texture(u_depthTex,uv + vec2(diff.x,0.0)).r;
	float depthX2 = texture(u_depthTex,uv - vec2(diff.x,0.0)).r;
	float depthY1 = texture(u_depthTex,uv + vec2(0.0,diff.y)).r;
	float depthY2 = texture(u_depthTex,uv - vec2(0.0,diff.y)).r;

	return 
		depth +
		depthX1 + depthX2 +
		depthY1 + depthY2;
}

float getNormalDiff(vec2 uv, vec2 diff)
{
	vec3 normal =   texture(u_normalTex,uv).rgb * 2.0 - 1.0;
	vec3 normalX1 = texture(u_normalTex,uv + vec2(diff.x,0.0)).rgb * 2.0 - 1.0;
	vec3 normalX2 = texture(u_normalTex,uv - vec2(diff.x,0.0)).rgb * 2.0 - 1.0;
	vec3 normalY1 = texture(u_normalTex,uv + vec2(0.0,diff.y)).rgb * 2.0 - 1.0;
	vec3 normalY2 = texture(u_normalTex,uv - vec2(0.0,diff.y)).rgb * 2.0 - 1.0;

	float d = 0.0;
    d += 1.0 - dot(normal, normalX1);
    d += 1.0 - dot(normal, normalX2);
    d += 1.0 - dot(normal, normalY1);
    d += 1.0 - dot(normal, normalY2);

    return d;
}

void main()
{
	float depthDiff = getDepthDiff(f_texcoord,u_texelSize);
	float normalDiff = getNormalDiff(f_texcoord,u_texelSize);
	if(normalDiff > u_diffNormal || depthDiff > u_diffDepth) {
		FragColor = vec4(0.0,0.0,0.0,1.0);
	}else{
		FragColor = texture(u_colorTex,f_texcoord);
	}
} 
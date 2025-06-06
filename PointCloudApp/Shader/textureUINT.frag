layout(location = 0)out vec4 FragColor;

uniform usampler2D tex;

vec3 idToColor(uint id)
{
	float normalizeId = float(id%256)/255.0;
	return vec3(normalizeId,0.0,1.0-normalizeId);
}
void main()
{
	ivec2 texCoords = ivec2(gl_FragCoord.xy);
	uint id = texture(tex,texCoords).r;
	
	FragColor = vec4(id,0.0,0.0,1.0);
} 

layout(std430,binding = 1) buffer TileLightBuffer
{
   TileLight[] tileLights;
};

layout(location = 0) out vec4 FragColor;
uniform ivec2 u_tileCount;
uniform ivec2 u_tileSize;
uniform int u_maxLightNum;
vec3 Heatmap(float t)
{
    t = clamp(t, 0.0, 1.0);
    if (t < 0.25) return mix(vec3(0, 0, 1), vec3(0, 1, 1), t * 4.0);
    if (t < 0.50) return mix(vec3(0, 1, 1), vec3(0, 1, 0), (t - 0.25) * 4.0);
    if (t < 0.75) return mix(vec3(0, 1, 0), vec3(1, 1, 0), (t - 0.50) * 4.0);
    return mix(vec3(1, 1, 0), vec3(1, 0, 0), (t - 0.75) * 4.0);
}



void main()
{
	ivec2 tile = ivec2(gl_FragCoord.xy) / u_tileSize;
	uint count = tileLights[tile.y * u_tileCount.x + tile.x].count;
	FragColor = vec4(Heatmap(float(count) / float(u_maxLightNum)),0.4);	
} 

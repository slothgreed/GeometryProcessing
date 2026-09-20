
layout(std430,binding = 1) buffer TileLightBuffer
{
   TileLight[] tileLights;
};

layout(location = 0) out vec4 FragColor;
uniform ivec2 u_tileCount;
uniform ivec2 u_tileSize;
uniform int u_maxLightNum;
uniform vec2 u_depthRange;
// Matches the UI list: minimum depth, maximum depth, light count.
uniform int u_displayMode;
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
	ivec2 pixel = ivec2(gl_FragCoord.xy);
	ivec2 localPixel = pixel % u_tileSize;
	// Draw only the left and bottom edges to keep shared borders one pixel wide.
	if (localPixel.x == 0 || localPixel.y == 0)
	{
		FragColor = vec4(0.6, 0.6, 0.6, 1.0);
		return;
	}
	ivec2 tile = pixel / u_tileSize;
	int index = tile.y * u_tileCount.x + tile.x;

	if (u_displayMode == 2)
	{
		int count = tileLights[index].count;
		FragColor = vec4(Heatmap(float(count) / float(u_maxLightNum)), 0.4);
		return;
	}

	float minDepth = tileLights[index].minDepth;
	float maxDepth = tileLights[index].maxDepth;
	// Tiles containing only background retain the initial min/max values.
	if (minDepth > maxDepth)
	{
		FragColor = vec4(0.08, 0.08, 0.08, 1.0);
		return;
	}

	// The resource bounding box projected onto the camera's view-space depth axis.
	float depth = u_displayMode == 0 ? minDepth : maxDepth;
	float depthSpan = max(u_depthRange.y - u_depthRange.x, 1e-5);
	float t = clamp((depth - u_depthRange.x) / depthSpan, 0.0, 1.0);
	FragColor = vec4(Heatmap(t), 0.4);
} 

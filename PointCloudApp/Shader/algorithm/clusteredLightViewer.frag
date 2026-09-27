
layout(std430,binding = 1) buffer ClusteredLightBuffer
{
   ClusteredLight[] clusteredLights;
};

layout(location = 0) out vec4 FragColor;
uniform ivec2 u_clusterCount;
uniform ivec3 u_clusterPartition;
uniform int u_maxLightNum;
// Matches the UI list: minimum depth, maximum depth, light count.
uniform int u_displayMode;
uniform int u_slice;
uniform bool u_showGrid;
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
	ivec2 localPixel = pixel % u_clusterPartition.xy;
	// Draw only the left and bottom edges to keep shared borders one pixel wide.
	if (u_showGrid && (localPixel.x == 0 || localPixel.y == 0))
	{
		FragColor = vec4(0.6, 0.6, 0.6, 1.0);
		return;
	}
	ivec2 cluster = pixel / u_clusterPartition.xy;
	int slice = clamp(u_slice, 0, u_clusterPartition.z - 1);
	int index = cluster.y * u_clusterCount.x + cluster.x
	          + slice * u_clusterCount.x * u_clusterCount.y;

	if (u_displayMode == 2)
	{
		int count = clusteredLights[index].count;
		FragColor = vec4(Heatmap(float(count) / float(u_maxLightNum)), 0.4);
		if (clusteredLights[index].count == 1) {
			FragColor = vec4(1.0, 0.0, 1.0, 0.7);
		}else{
			FragColor = vec4(0.0,0.0,1.0,0.4);
		}
		return;
	}

	float minDepth = clusteredLights[index].minDepth;
	float maxDepth = clusteredLights[index].maxDepth;
	if (minDepth > maxDepth)
	{
		FragColor = vec4(0.08, 0.08, 0.08, 1.0);
		//return;
	}
	if (u_displayMode == 0){
		FragColor = vec4(minDepth,minDepth,minDepth, 0.9);
	}else if (u_displayMode == 1){
		FragColor = vec4(maxDepth,maxDepth,maxDepth, 0.9);
	}
} 

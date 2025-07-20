struct Meshlet
{
	int offset;
	int size;
	int meshletIndex;
	int padding;
	vec4 boxMin;
	vec4 boxMax;
	vec4 normal;
};

bool allCull(vec3 meshletMin, vec3 meshletMax, mat4 MVP, vec2 screenResolution, float pixelThreshold)
{
    vec3 corners[8] = vec3[](
        vec3(meshletMin.x, meshletMin.y, meshletMin.z),
        vec3(meshletMax.x, meshletMin.y, meshletMin.z),
        vec3(meshletMin.x, meshletMax.y, meshletMin.z),
        vec3(meshletMax.x, meshletMax.y, meshletMin.z),
        vec3(meshletMin.x, meshletMin.y, meshletMax.z),
        vec3(meshletMax.x, meshletMin.y, meshletMax.z),
        vec3(meshletMin.x, meshletMax.y, meshletMax.z),
        vec3(meshletMax.x, meshletMax.y, meshletMax.z)
    );

    vec2 minScreen = vec2(1e10);
    vec2 maxScreen = vec2(-1e10);
    bool outside = true;

    for (int i = 0; i < 8; ++i)
    {
        vec4 clip = MVP * vec4(corners[i], 1.0);

        if (abs(clip.x) <= clip.w &&
            abs(clip.y) <= clip.w &&
            clip.z >= 0.0 && clip.z <= clip.w)
        {
            outside = false;
        }

        vec2 ndc = clip.xy / clip.w;
        vec2 screenPos = (ndc * 0.5 + 0.5) * screenResolution;

        minScreen = min(minScreen, screenPos);
        maxScreen = max(maxScreen, screenPos);
    }

    if (outside)
        return true; 

    vec2 size = maxScreen - minScreen;
    float area = size.x * size.y;

    if (area < pixelThreshold)
        return true; 

    return false;
}
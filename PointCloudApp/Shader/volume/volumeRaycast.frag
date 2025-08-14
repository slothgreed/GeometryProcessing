layout(location = 0)out vec4 FragColor;

in vec3 fragPos;
uniform sampler3D volumeTex;
uniform float u_stepSize;
uniform mat4x4 u_Model;
layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

layout(std430, binding = 1) buffer VoxelBuffer
{
	Voxel voxel;
};




bool intersectBox(vec3 rayOrigin, vec3 rayDir, vec3 bdbMin, vec3 bdbMax, out float tNear, out float tFar) {
    vec3 invDir = 1.0 / rayDir;
    vec3 t0 = (bdbMin - rayOrigin) * invDir;
    vec3 t1 = (bdbMax - rayOrigin) * invDir;
    vec3 tMin = min(t0, t1);
    vec3 tMax = max(t0, t1);
    tNear = max(max(tMin.x, tMin.y), tMin.z);
    tFar  = min(min(tMax.x, tMax.y), tMax.z);
    return tFar >= tNear;
}


void main() {
    // スクリーン座標をワールドレイに変換
    vec2 screenPos = fragPos.xy * 2.0 - 1.0;
    vec4 ndcNear = vec4(screenPos, 0.0, 1.0);
    vec4 ndcFar  = vec4(screenPos, 1.0, 1.0);

    vec4 worldNear = camera.invVP * ndcNear;
    vec4 worldFar  = camera.invVP * ndcFar;
    worldNear /= worldNear.w;
    worldFar  /= worldFar.w;

    vec3 rayOrigin = worldNear.xyz;
    vec3 rayDir = normalize(worldFar.xyz - worldNear.xyz);
	vec3 bdbMin = (u_Model * vec4(voxel.bdbMin.xyz,1.0)).xyz;
	vec3 bdbMax = (u_Model * vec4(voxel.bdbMax.xyz,1.0)).xyz;
	vec3 pitch = (u_Model * vec4(voxel.pitch.xyz,1.0)).xyz;
    float tMin, tMax;
    if (!intersectBox(rayOrigin, rayDir, bdbMin, bdbMax, tMin, tMax)) {
		discard;
    }
	
    float stepSize = min(min(pitch.x, pitch.y), pitch.z);
    int maxSteps = int((tMax - tMin) / stepSize);
    vec4 accumulatedColor = vec4(0.0);
	vec3 worldPos = rayOrigin;
	vec3 worldDir = rayDir;
	vec3 invBDBLength = 1 / (bdbMax.xyz - bdbMin.xyz);
	vec3 stepPos = worldPos + worldDir * tMin;
    for (int i = 0; i < maxSteps; ++i) {
		vec3 local = (stepPos - bdbMin.xyz) * invBDBLength;

		vec4 col = vec4(texture(volumeTex, local).r);
		
        // Pre-multiplied alpha blending
        accumulatedColor.rgb += (1.0 - accumulatedColor.a) * col.rgb * col.a;
        accumulatedColor.a += (1.0 - accumulatedColor.a) * col.a;

        if (accumulatedColor.a >= 0.95) break; 

        stepPos += worldDir * stepSize;
        if (stepPos.z > tMax) { break; }
    }
	
	if(accumulatedColor.a < 0.01){discard;}
    FragColor = accumulatedColor;
}
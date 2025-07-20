in vec2 f_texcoord;
layout(location = 0)out vec4 FragColor;

uniform int uMipmap;
uniform samplerCube uCubemap;

vec3 getDirectionFromLayout(vec2 uv) {
    vec2 tileUV = fract(uv * vec2(4.0, 3.0));        // タイル内のUV（0~1）
    vec2 tileIdx = floor(uv * vec2(4.0, 3.0));       // タイルインデックス

    vec3 dir = vec3(0.0);

    if (tileIdx == vec2(2,1)) {        // +X
        dir = vec3( 1.0, -tileUV.y * 2.0 + 1.0, -tileUV.x * 2.0 + 1.0);
    } else if (tileIdx == vec2(0,1)) { // -X
        dir = vec3(-1.0, -tileUV.y * 2.0 + 1.0,  tileUV.x * 2.0 - 1.0);
    } else if (tileIdx == vec2(1,0)) { // +Y
        dir = vec3( tileUV.x * 2.0 - 1.0, 1.0,  tileUV.y * 2.0 - 1.0);
    } else if (tileIdx == vec2(1,2)) { // -Y
        dir = vec3( tileUV.x * 2.0 - 1.0, -1.0, -tileUV.y * 2.0 + 1.0);
    } else if (tileIdx == vec2(1,1)) { // +Z
        dir = vec3( tileUV.x * 2.0 - 1.0, -tileUV.y * 2.0 + 1.0, 1.0);
    } else if (tileIdx == vec2(3,1)) { // -Z
        dir = vec3(-tileUV.x * 2.0 + 1.0, -tileUV.y * 2.0 + 1.0, -1.0);
    } else {
        dir = vec3(0,0,0);
    }

	return normalize(vec3(dir.x,-dir.y,dir.z));
}

void main() {
    vec3 dir = getDirectionFromLayout(f_texcoord);
    FragColor = textureLod(uCubemap, dir, uMipmap);
}
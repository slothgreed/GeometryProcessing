layout(location = 0) out vec4 FragColor;

in vec4 f_color;
in vec3 f_position;

uniform vec4 u_Plane;
uniform int u_HatchEnabled;
uniform vec3 u_HatchColor;
uniform float u_HatchSpacing;
uniform float u_HatchLineWidth;

void main()
{
    if (u_HatchEnabled == 0 || u_HatchSpacing <= 0.0) {
        FragColor = f_color;
        return;
    }

    vec3 normal = normalize(u_Plane.xyz);
    vec3 reference = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    vec3 tangent = normalize(cross(reference, normal));
    vec3 bitangent = cross(normal, tangent);
    vec2 planePosition = vec2(dot(f_position, tangent), dot(f_position, bitangent));

    float hatchCoordinate = (planePosition.x + planePosition.y) / u_HatchSpacing;
    float distanceToLine = abs(fract(hatchCoordinate) - 0.5);
    float antialiasWidth = fwidth(hatchCoordinate);
    float background = smoothstep(
        u_HatchLineWidth,
        u_HatchLineWidth + antialiasWidth,
        distanceToLine);

    FragColor = vec4(mix(u_HatchColor, f_color.rgb, background), f_color.a);
}

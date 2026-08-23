layout(location = 0) in vec3 position;

uniform mat4 u_Model;
uniform vec4 u_Plane;
uniform vec3 u_Color;

layout(std430, binding = 0) buffer CameraBuffer
{
    Camera camera;
};

out vec4 f_color;
out vec3 f_position;

void main()
{
    gl_Position = camera.VP * u_Model * vec4(position, 1.0);
    gl_ClipDistance[0] = dot(vec4(position, 1.0), u_Plane);
    f_color = vec4(u_Color, 1.0);
    f_position = position;
}

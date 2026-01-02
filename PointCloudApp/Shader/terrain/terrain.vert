layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;
out vec2 tc_texCoord;
out flat int tc_InstanceID;
void main()
{
	gl_Position = vec4(position, 1.0);
	tc_texCoord = texcoord;
    tc_InstanceID = gl_InstanceID;
}
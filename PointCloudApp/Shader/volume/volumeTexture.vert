layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;

layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};

uniform mat4 u_Model;
uniform vec4 u_Position; // axis, position;

out vec3 f_texCoord;
void main()
{
    vec4 pos = camera.VP * u_Model * vec4(position.x,position.y,position.z, 1.0);
	gl_Position = pos;
	
	int axis = int(u_Position.x + 0.5);
	float slice = u_Position.y;
	
	if (axis == 0) {
		f_texCoord = vec3(slice, texcoord.y, texcoord.x);
	} else if (axis == 1) {
		f_texCoord = vec3(texcoord.y, slice, texcoord.x);
	} else if (axis == 2) {
		f_texCoord = vec3(texcoord.x, texcoord.y, slice); 
	}else{
		f_texCoord = vec3(0,0,0);
	}
}
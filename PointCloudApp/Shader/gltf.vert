layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec4 tangent;
layout (location = 4) in vec4 joint;
layout (location = 5) in vec4 weight;


layout(std430, binding = 4) buffer NodeBuffer
{
	GLTFNode nodes[];
};

uniform mat4 u_VP;
uniform ivec2 u_ssboIndex; // (x,y,z,w) = (matrix,material,Hoge,Hoge);

out vec3 f_normal;
out vec2 f_texcoord;
out vec4 f_tangent;
void main()
{
	/*
	if (node.jointCount > 0) {
		mat4 skinMatrix =
			weight.x * jointMatrix[int(joint.x)] +
			weight.y * jointMatrix[int(joint.y)] +
			weight.z * jointMatrix[int(joint.z)] +
			weight.w * jointMatrix[int(joint.w)];
		gl_Position = u_VP * nodes[u_ssboIndex.x].matrix * skinMatrix * vec4(position.x, position.y, position.z, 1.0);
	} else 
	*/
	{
		gl_Position = u_VP * nodes[u_ssboIndex.x].matrix * vec4(position.x, position.y, position.z, 1.0);
	}
    f_normal = normal;
	f_texcoord = texcoord;
	f_tangent = tangent;
}
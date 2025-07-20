layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec4 tangent;
layout (location = 4) in vec4 joint;
layout (location = 5) in vec4 weight;

layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};


layout(std430, binding = 4) buffer NodeBuffer
{
	GLTFNode nodes[];
};


layout(std430, binding = 5) buffer SkinBuffer
{
	float skins[];
};


uniform mat4 u_VP;
uniform mat4 u_Model;
uniform ivec2 u_ssboIndex; // (x,y,z,w) = (matrix,material,Hoge,Hoge);

out vec3 f_worldPos;
out vec3 f_normal;
out vec2 f_texcoord;
out vec4 f_tangent;
void main()
{
	mat4 worldMat = u_Model * nodes[u_ssboIndex.x].matrix;
	if (nodes[u_ssboIndex.x].skinId >= 0) {
	
		/*	
		mat4 skinMatrix =
			weight.x * jointMatrix[int(joint.x)] +
			weight.y * jointMatrix[int(joint.y)] +
			weight.z * jointMatrix[int(joint.z)] +
			weight.w * jointMatrix[int(joint.w)];
		*/

		mat4 skinMatrix =
			weight.x * GetJointMatrix(skins,0,int(joint.x)) +
			weight.y * GetJointMatrix(skins,0,int(joint.y)) +
			weight.z * GetJointMatrix(skins,0,int(joint.z)) +
			weight.w * GetJointMatrix(skins,0,int(joint.w));

		worldMat *= skinMatrix;
	} 
	
	f_worldPos = (worldMat * vec4(position.x,position.y,position.z,1.0)).xyz;
	f_normal = normalize(transpose(inverse(mat3(worldMat))) * normal);
	f_texcoord = texcoord;
	f_tangent = tangent;
	gl_Position = camera.VP * vec4(f_worldPos,1.0);
}
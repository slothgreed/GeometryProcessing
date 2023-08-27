#version 420
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in int matrixIndex;
layout (binding=0) uniform samplerBuffer matrixTex;
uniform mat4 u_VP;
uniform mat4 u_Model;

out vec4 f_color;
void main()
{
	int index = matrixIndex * 4;
	mat4 modelMat = mat4(
		texelFetch(matrixTex,index + 0),
		texelFetch(matrixTex,index + 1),
		texelFetch(matrixTex,index + 2),
		texelFetch(matrixTex,index + 3)
	);
    gl_Position = u_VP * modelMat * vec4(position.x, position.y, position.z, 1.0);
	f_color = vec4(normal,1.0);
}
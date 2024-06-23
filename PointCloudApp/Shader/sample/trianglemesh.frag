#version 450
#extension GL_NV_fragment_shader_barycentric : enable
out vec4 FragColor;

void main()
{
	FragColor = vec4(gl_BaryCoordNV,1.0);
}
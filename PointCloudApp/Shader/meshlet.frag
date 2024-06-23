#version 450
#extension GL_NV_fragment_shader_barycentric : enable
out vec4 FragColor;

layout (location = 0) in PerVertexData
{
  vec4 color;
};

void main()
{
	FragColor = color;
}
out vec4 FragColor;
layout(std430, binding = 1) buffer colorBuffer
{
	vec4 color[];
};
void main()
{
    FragColor = color[gl_PrimitiveID];
} 
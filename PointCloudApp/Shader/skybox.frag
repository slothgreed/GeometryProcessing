layout(location = 0)out vec4 FragColor;
in vec3 texCoord;
uniform samplerCube u_skyBox;
void main()
{
	FragColor = texture(u_skyBox,texCoord);
} 

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 PositionTexture;
layout(location = 2)out vec4 NormalTexture;

in vec4 f_color;
in vec3 f_normal;
in vec4 f_position;
void main()
{
    FragColor = f_color;
	PositionTexture = f_position;
	NormalTexture = vec4((f_normal.xyz + 1) * 0.5f, 1.0f);
} 
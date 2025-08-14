layout(location = 0)out vec4 FragColor;
in vec3 f_texCoord;
uniform sampler3D u_texture3D;
void main()
{
	vec4 tex = texture(u_texture3D,f_texCoord);
	FragColor = vec4(tex.rgb,1.0);
} 
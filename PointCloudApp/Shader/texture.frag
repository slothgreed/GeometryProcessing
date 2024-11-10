out vec4 FragColor;
in vec2 f_texcoord;
uniform sampler2D tex;
void main()
{
    FragColor = vec4(texture(tex,f_texcoord).rgb,1.0);
} 
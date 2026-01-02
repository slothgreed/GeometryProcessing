layout(location = 0)out vec4 FragColor;
in vec2 f_texcoord;
uniform sampler2D u_texture;
uniform int u_showWire;
void main()
{
   if (u_showWire == 1){
        FragColor = vec4(1, 0, 0, 1);
	} else{
		FragColor = vec4(texture(u_texture,f_texcoord).rgb,1.0);
	}
} 
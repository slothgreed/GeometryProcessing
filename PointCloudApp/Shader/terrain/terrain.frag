layout(location = 0)out vec4 FragColor;
in vec2 f_texcoord;

uniform sampler2D u_texture;
uniform int u_showWire;
in flat int f_tessLevel;

void main()
{
	if (u_showWire == 1) {
		if(f_tessLevel == 4){
			FragColor = vec4(1, 0, 0, 1);
		} else if(f_tessLevel == 3) {
			FragColor = vec4(0, 1, 0, 1);
		} else if(f_tessLevel == 2) {
			FragColor = vec4(0, 0, 1, 1);
		} else if(f_tessLevel == 1) {
			FragColor = vec4(1, 1, 0, 1);
		} else if(f_tessLevel == 0) {
			FragColor = vec4(0, 1, 1, 1);
		}
	} else{
		FragColor = vec4(texture(u_texture,f_texcoord).rgb,1.0);
	}
} 
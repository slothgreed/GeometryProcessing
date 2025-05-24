
layout (binding = 0) uniform sampler2D u_colorForward;
layout (binding = 1) uniform sampler2D u_depthForward;

layout (binding = 2) uniform usampler2D u_colorCompute;
layout (binding = 3) uniform usampler2D u_depthCompute;

in vec2 f_texcoord;

out vec4 FragColor;

void main()
{
	float depthCompute = uintBitsToFloat(texture(u_depthCompute,f_texcoord).r);
	float depthForward = texture(u_depthForward,f_texcoord).r;
	
	if(depthForward < depthCompute){
		FragColor = texture(u_colorForward,f_texcoord);
	}else{
		FragColor = unpackColor(uintBitsToFloat(texture(u_colorCompute,f_texcoord).r));
	}
}
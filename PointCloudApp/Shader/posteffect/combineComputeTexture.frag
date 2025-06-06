
layout (binding = 0) uniform sampler2D u_colorForward;
layout (binding = 1) uniform sampler2D u_depthForward;

layout(std430, binding = 2) buffer colorImageBuffer
{
	uint u_colorCompute[];
};

layout(std430, binding = 3) buffer depthImageBuffer
{
	uint u_depthCompute[];
};

in vec2 f_texcoord;
uniform ivec2 u_ImageSize;
layout(location = 0)out vec4 FragColor;

void main()
{
	ivec2 pixel = ivec2(gl_FragCoord.xy);
    uint index = uint(pixel.y * u_ImageSize.x + pixel.x);
	float depthCompute = uintBitsToFloat(u_depthCompute[index]);
	float depthForward = texture(u_depthForward,f_texcoord).r;
	
	if(depthForward < depthCompute){
		FragColor = texture(u_colorForward,f_texcoord);
	}else{
		FragColor = unpackColor(uintBitsToFloat(u_colorCompute[index]));
	}
}
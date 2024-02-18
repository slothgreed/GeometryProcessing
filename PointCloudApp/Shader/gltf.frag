#version 400
out vec4 FragColor;
in vec2 f_texcoord;
in vec4 f_tangent;
in vec3 f_normal; 
uniform sampler2D u_colorTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_roughnessTexture;

void main()
{
    FragColor = vec4(texture(u_roughnessTexture,f_texcoord).rgb,1.0);
} 
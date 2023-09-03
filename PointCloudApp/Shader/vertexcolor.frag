#version 400
out vec4 FragColor;
in vec4 f_color;
void main()
{
    FragColor = f_color;
    //FragColor = vec4(1,1,0,1);
} 
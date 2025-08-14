layout (location = 0) in vec3 position;

out vec3 fragPos;

void main()
{
    fragPos = position * 0.5 + 0.5; 
	gl_Position = vec4(position, 1.0);
}
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;


out VS_OUT {
	vec3 position;
	vec3 vector;
} gout;

void main()
{
    gout.position = position;
	gout.vector = normal;
}
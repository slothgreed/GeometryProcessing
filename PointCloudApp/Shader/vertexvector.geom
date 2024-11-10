layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform float u_Length;
uniform mat4 u_Model;


layout(std430, binding = 0) buffer CameraBuffer
{
	Camera camera;
};


in VS_OUT
{
	vec3 position;
	vec3 vector;
}gin[1];



void main()
{
	gl_Position = camera.VP * u_Model * vec4(gin[0].position,1.0);
	EmitVertex();
	gl_Position = camera.VP * u_Model * vec4(gin[0].position + gin[0].vector * u_Length, 1.0);
	EmitVertex();
	
	EndPrimitive();
}
out vec4 FragColor;
uniform uint u_PickOffset;
void main()
{
	uint primId = gl_PrimitiveID + u_PickOffset;
    float r = float((primId >> 24) & 0xFF) / 255.0; 
    float g = float((primId >> 16) & 0xFF) / 255.0; 
    float b = float((primId >> 8) & 0xFF) / 255.0;  
    float a = float(primId & 0xFF) / 255.0;         

    FragColor = vec4(r, g, b, a);
} 
out vec4 FragColor;
uniform uint u_PickOffset;
void main()
{
#ifdef PICK_BY_PRIMITIVE
	uint pickID = gl_PrimitiveID + u_PickOffset;
#elif defined(PICK_BY_ID)
	uint pickID = u_PickOffset;
#endif
    float r = float((pickID >> 24) & 0xFF) / 255.0; 
    float g = float((pickID >> 16) & 0xFF) / 255.0; 
    float b = float((pickID >> 8) & 0xFF) / 255.0;  
    float a = float(pickID & 0xFF) / 255.0;         

    FragColor = vec4(r, g, b, a);
} 
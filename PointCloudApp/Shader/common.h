struct Camera
{
	mat4x4 view;
	mat4x4 proj;
	mat4x4 VP;
	mat4x4 invVP;
	vec4 eye;
	vec4 center;
	vec2 viewSize;
	float padding[2];
	vec4 frustum[6];
};

struct Light
{
	vec4 color;
	vec4 direction;
	float padding[56];
};

struct DrawElementsIndirect
{
    uint count;
    uint instanceCount;
    uint firstIndex;
    int baseVertex;
    uint baseInstance;
};

struct DrawArrayIndirect
{
    uint count;
    uint instanceCount;
    uint first;
    uint baseInstance;
};


vec4 tonemap(vec4 color)
{
	vec4 x = max(vec4(0),color * 16 - 0.004);
	return (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
}

vec4 IntToVec4(int value)
{
	float r = float((value >> 24) & 0xFF) / 255.0;
    float g = float((value >> 16) & 0xFF) / 255.0;
    float b = float((value >> 8) & 0xFF) / 255.0;
    float a = float(value & 0xFF) / 255.0;
    return vec4(r, g, b, a);
}
uint packColor(vec4 color)
{
    uvec4 c = uvec4(round(clamp(color, 0.0, 255.0)));

    return (c.r << 24u)
         | (c.g << 16u)
         | (c.b << 8u)
         |  c.a;
}

uint packColor(vec3 color)
{
    return packColor(vec4(color,255.0));
}

vec3 unpackColor3F(float f)
{
    vec3 color;
    color.r = floor(f / 65536.0); // 256 * 256
    f -= color.r * 65536.0;
    color.g = floor(f / 256.0);
    f -= color.g * 256.0;
    color.b = f;
    return vec3(color / 255.0);
}

vec4 unpackColor4(uint value)
{
    vec4 c;
    c.r = float((value >> 24u) & 255u);
    c.g = float((value >> 16u) & 255u);
    c.b = float((value >> 8u)  & 255u);
    c.a = float( value        & 255u);

    return c / 255.0;
}

bool IsSame(float lhs, float rhs) {
    return abs(lhs - rhs) < 1e-5;
}

mat3 quatToMat3(vec4 q)
{
    q = normalize(q);

    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;

    return mat3(
        1.0 - 2.0*y*y - 2.0*z*z, 2.0*x*y + 2.0*w*z,       2.0*x*z - 2.0*w*y,
        2.0*x*y - 2.0*w*z,       1.0 - 2.0*x*x - 2.0*z*z, 2.0*y*z + 2.0*w*x,
        2.0*x*z + 2.0*w*y,       2.0*y*z - 2.0*w*x,       1.0 - 2.0*x*x - 2.0*y*y
    );
}

vec2 projectToScreen(mat4x4 vp, vec2 imageSize, vec3 p)
{
    vec4 clip = vp * vec4(p, 1.0);
    vec3 ndc = clip.xyz / clip.w;
    vec2 screen = ndc.xy * 0.5 + 0.5;
    return screen * imageSize;
}

struct ScreenPos
{
	bool inner;
    vec2 pixel;
    float depth;
    float w;
};

ScreenPos toScreen(mat4x4 vp, ivec2 imageSize, vec3 modelPos)
{
	ScreenPos screen;
	screen.inner = false;
    vec4 clip = vp * vec4(modelPos, 1.0);

    if (clip.w <= 0.0){return screen;}

    vec4 pos = clip / clip.w;

    if (pos.x < -1.0 || pos.x > 1.0 ||
        pos.y < -1.0 || pos.y > 1.0)
    {
		return screen;
    }

    pos = pos * 0.5 + 0.5;
    screen.inner = true; 
    screen.pixel = ivec2(pos.xy * vec2(imageSize));;
	screen.depth = pos.z;
	screen.w = clip.w;
    return screen;
}

bool inScreen(ivec2 screen, ivec2 imageSize)
{
	return 
		screen.x >= 0 && screen.x < imageSize.x &&
		screen.y >= 0 && screen.y < imageSize.y;		
}

struct Ellipse
{
	vec2 center;
	vec2 axisX;
	vec2 axisY;
	vec3 conic;
};


Ellipse getEllipse(mat4x4 vp, ivec2 imageSize,vec3 scale, vec4 quart, vec3 center)
{
    Ellipse ellipse;
	mat3 R = quatToMat3(quart);

    vec2 c  = projectToScreen(vp,imageSize,center);
    vec2 px = projectToScreen(vp,imageSize,center + R[0] * scale.x);
    vec2 py = projectToScreen(vp,imageSize,center + R[1] * scale.y);
  	ellipse.center = c;
	ellipse.axisX = px - c;
    ellipse.axisY = py - c;
	
	mat2 A = mat2( ellipse.axisX, ellipse.axisY);
	mat2 cov = A * transpose(A);
	float det = determinant(cov);
	if(abs(det) < 1e-6)
	{
		ellipse.conic = vec3(1e30, 0.0, 1e30);
	}
	else
	{
		mat2 invCov = inverse(cov);
		ellipse.conic = vec3(invCov[0][0], invCov[0][1], invCov[1][1]);
	}

	return ellipse;
}

float getEllipseR2(vec2 d, Ellipse ellipse)
{
    return
        ellipse.conic.x * d.x * d.x +
        2.0 * ellipse.conic.y * d.x * d.y +
        ellipse.conic.z * d.y * d.y;
}
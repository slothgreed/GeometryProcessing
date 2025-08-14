vec3 GRAVITY = vec3(0.0f,-9.8f,0.0f);
float DT = 0.01f;
float RADIUS = 2;
float RADIUS2 = 2 * 2;
float PRESSURE_STRENGTH = 200.0f;
float VISCOSITY = 0.1f;
#define PI 3.14159265359

struct Setting
{
	ivec4 cellNum;
	vec4 spaceMin;
	vec4 spaceMax;
	int particleNum;
	int gridNum;
	int pad1;
	int pad2;
};

struct Particle
{
	vec4 position;
	vec4 velocity;
};


struct Grid
{
	uint offset;
	uint num;
	float weight;
	uint padding;
};


layout(std430, binding = 0) buffer settingBuffer
{
	Setting setting;
};

layout(std430, binding = 1) buffer particleBuffer
{
	Particle particles[];
};

layout(std430, binding = 2) buffer gridBuffer
{
	Grid grids[];
};

layout(std430, binding = 3) buffer sortParticleBuffer
{
	uint sortParticles[];
};

vec3 getPosition(Particle p)
{
	return vec3(p.position.x,p.position.y,p.position.z);
}

vec3 getVelocity(Particle p)
{
	return vec3(p.velocity.x,p.velocity.y,p.velocity.z);
}


void addPosition(inout Particle p, vec3 pos)
{
	p.position.x += pos.x;
	p.position.y += pos.y;
	p.position.z += pos.z;
}

void mulPosition(inout Particle p, vec3 pos)
{
	p.position.x *= pos.x;
	p.position.y *= pos.y;
	p.position.z *= pos.z;
}

void addVelocity(inout Particle p, vec3 pos)
{
	p.velocity.x += pos.x;
	p.velocity.y += pos.y;
	p.velocity.z += pos.z;
}

void mulVelocity(inout Particle p, vec3 pos)
{
	p.velocity.x *= pos.x;
	p.velocity.y *= pos.y;
	p.velocity.z *= pos.z;
}


ivec3 getCellID(vec3 pos)
{
    vec3 position = pos - setting.spaceMin.xyz;
    return ivec3(floor(position / RADIUS));
}


vec3 getCellCenter(ivec3 id)
{
    return vec3(setting.spaceMin.xyz + (vec3(id.x, id.y, id.z) + vec3(0.5f)) * RADIUS);
}


int ToIndex1D(ivec3 cellID)
{
    return
        cellID.z * setting.cellNum.y * setting.cellNum.x +
        cellID.y * setting.cellNum.x +
        cellID.x;
}

uint getOffset(Grid g)
{
	return g.offset;
}

void setOffset(inout Grid g, uint offset)
{
	g.offset = offset;
}

void addNum(inout Grid g, uint index)
{
	g.num += 1;
}

uint getNum(Grid g)
{
	return g.num;
}

float getWeight(Grid g)
{
	return g.weight;
}

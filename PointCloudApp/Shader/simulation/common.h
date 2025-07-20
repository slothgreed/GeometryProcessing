vec3 GRAVITY = vec3(0.0f,-9.8f,0.0f);
float DT = 0.01f;
float RADIUS = 2;
float RADIUS2 = 2 * 2;
float PRESSURE_STRENGTH = 200.0f;
float VISCOSITY = 0.1f;
#define PI 3.14159265359

uniform vec3i u_cellNum;
uniform vec3 u_spaceMin;
uniform vec3 u_spaceMax;
uniform int u_particleNum;

struct Setting
{
	vec4i cellNum;
	vec4 spaceMin;
	vec4 spaceMax;
	int particleNum;
}

struct Particle
{
	vec4 position;
	vec4 velocity;
}

layout(std430, binding = 0) buffer settingBuffer
{
	Setting setting;
}

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
}

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
    vec3 position = pos - u_spaceMin;
    return ivec3(glm::floor(position / RADIUS));
}


vec3 getCellCenter(ivec3 id)
{
    return Vector3(u_spaceMin + (vec3(id.x, id.y, id.z) + vec3(0.5f)) * RADIUS);
}


int ToIndex1D(ivec3 cellID)
{
    return
        cellID.z * u_cellNum.y * u_cellNum.x +
        cellID.y * u_cellNum.x +
        cellID.x;
}


struct Grid
{
	uint offset;
	uint num;
	uint weight;
	uint padding;
}

uint getOffset(Grid g)
{
	return g.offset;
}

void setOffset(inout Grid g, uint offset)
{
	g.offset = offset;
}

uint getNum(Grid g)
{
	return g.num;
}

uint getWeight(Grid g)
{
	return uintBitsToFloat(g.weight);
}

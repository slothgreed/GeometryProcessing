#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "IPrimitive.h"
class Cube : public IPrimitive
{
public:
	Cube(const vec3& min, const vec3& max);
	~Cube() {};

private:
	vec3 m_min;
	vec3 m_max;

	void AddIndex(int index, int vertex0, int vertex1, int vertex2, int vertex3);

};

class Cone : public IPrimitive
{
public:
	Cone(float _radius, float _height, int _partition);
	~Cone() {};

private:
	// radius = x around;
	// height = y value;
	float radius;
	float height;
	int partition;

};

class Cylinder : public IPrimitive
{
public:
	Cylinder(float _baseRad, float _topRad, float _height, int _slices);
	~Cylinder() {};

private:
	float baseRad;
	float topRad;
	float height;
	int slices;

};

class Sphere : public IPrimitive
{
public:
	Sphere(float _radius, int _slices, int _stacks);
	~Sphere() {};


private:
	float radius;
	int slices;	// z around;
	int stacks;	// z along;
};

class Torus : public IPrimitive
{
public:
	Torus(float _inRad, float _outRad, int _nsides, int _rings);
	~Torus() {};
private:

	float inRad;
	float outRad;
	int nsides;
	int rings;
};


class Triangle : public IPrimitive
{
public:
	Triangle();
	~Triangle();

	void Build();
private:

};

#endif PRIMITIVE_H

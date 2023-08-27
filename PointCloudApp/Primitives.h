#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "Primitive.h"
class Cube : public Primitive
{
public:
	Cube(const vec3& min, const vec3& max);
	~Cube() {};

private:
	vec3 m_min;
	vec3 m_max;

	void AddIndex(int index, int vertex0, int vertex1, int vertex2, int vertex3);

};

class Cone : public Primitive
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

class Cylinder : public Primitive
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

class Sphere : public Primitive
{
public:
	Sphere(float _radius, int _slices, int _stacks);
	~Sphere() {};


private:
	float radius;
	int slices;	// z around;
	int stacks;	// z along;
};

class Torus : public Primitive
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


class Triangle : public Primitive
{
public:
	Triangle();
	~Triangle();

	void Build();
private:

};

#endif PRIMITIVE_H

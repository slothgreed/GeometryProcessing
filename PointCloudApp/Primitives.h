#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "Primitive.h"
namespace KI
{
class Cube : public Primitive
{
public:
	Cube(const Vector3& min, const Vector3& max);
	~Cube() {};

private:
	Vector3 m_min;
	Vector3 m_max;


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

class Axis : public Primitive
{
public:
	Axis(float size);
	~Axis();

private:
	void Build(float size);
};

class Circle : public Primitive
{
public:
	Circle(float radius, const Vector3& center);
	Circle(float radius, int pointNum);

	~Circle();

private:
	void Build(float radius, int pointNum, const Vector3& center);
};

class RenderPlane : public Primitive
{
public:
	RenderPlane();
	~RenderPlane() {};

private:
	void Build();
};

class SkyBox : public Primitive
{
public:
	SkyBox();
	~SkyBox() {};

private:
};

}

#endif PRIMITIVE_H

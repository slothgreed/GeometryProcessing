#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "Primitive.h"
#include "Polyline.h"
namespace KI
{
class Cube : public Primitive
{
public:
	Cube() {};
	Cube(const Vector3& min, const Vector3& max);
	~Cube() {};


	static Cube CreateLine(const Vector3& min, const Vector3& max);
private:
	Vector3 m_min;
	Vector3 m_max;
};

class Plane : public Primitive
{
public:

	enum Axis
	{
		X,Y,Z
	};
	Plane() {}
	Plane(const Vector3& min, const Vector3& max, float position, Axis axis, bool texcoord = false);
	~Plane() {};
	
	static Matrix4x4 CreateMatrix(const Vector3& min, const Vector3& max, float position, Axis axis);

private:

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

	struct Mesh
	{
		Vector<Vector3> triangles;
		Polyline polyline;
	};

	static Mesh CreateMeshs(const Vector3& baseCenter, const Vector3& axis, float radius, float height, int slices, int stacks);

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

	static Polyline CreateLine(float radius, int pointNum, const Vector3& u, const Vector3& v, const Vector3& center);
	static Polyline CreateArc(float radius, int pointNum, const Vector3& u, const Vector3& v, const Vector3& center, const Vector3& begin, const Vector3& end);

private:
	void Build(float radius, int pointNum, const Vector3& center);
};

class RenderPlane : public Primitive
{
public:
	RenderPlane();
	~RenderPlane() {};

	static GLuint GetPrimitiveType();
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

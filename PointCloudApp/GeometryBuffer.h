#ifndef GEOMETRY_BUFFER_H
#define GEOMETRY_BUFFER_H
#include "GLBuffer.h"
namespace KI
{
class GeometryBuffer
{
public:
	GeometryBuffer() {};
	virtual ~GeometryBuffer() {};
private:
};


class BasicGeometryBuffer : public GeometryBuffer
{
public:
	BasicGeometryBuffer();
	~BasicGeometryBuffer();

	void SetPosition(const Vector<Vector3>& position);
	bool HasPosition() const { return m_pPosition != nullptr; }
	void SetNormal(const Vector<Vector3>& normal);
	bool HasNormal() const { return m_pNormal != nullptr; }
	void SetColor(const Vector<Vector3>& color);
	bool HasColor() const { return m_pColor != nullptr; }
	void SetTexcoord(const Vector<Vector2>& texture);
	bool HasTexcoord() const { return m_pTexcoord != nullptr; }
	void SetIndex(const Vector<int>& index);
	bool HasIndex() const { return m_pIndex != nullptr; }
private:
	Unique<GLBuffer> m_pPosition;
	Unique<GLBuffer> m_pNormal;
	Unique<GLBuffer> m_pColor;
	Unique<GLBuffer> m_pTexcoord;
	Unique<GLBuffer> m_pIndex;
};


}

#endif GEOMETRY_BUFFER_H
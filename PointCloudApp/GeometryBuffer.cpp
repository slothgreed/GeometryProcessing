#include "GeometryBuffer.h"
namespace KI
{
BasicGeometryBuffer::BasicGeometryBuffer()
	:m_pPosition(nullptr)
	,m_pNormal(nullptr)
	,m_pColor(nullptr)
	,m_pTexcoord(nullptr)
{
}

BasicGeometryBuffer::~BasicGeometryBuffer()
{
}

void BasicGeometryBuffer::SetPosition(const Vector<Vector3>& position)
{
	if (!m_pPosition) {
		m_pPosition = std::make_unique<GLBuffer>();
		m_pPosition->Create(position);
	} else {
		m_pPosition->BufferSubData(0, position);
	}
}

void BasicGeometryBuffer::SetNormal(const Vector<Vector3>& normal)
{
	if (!m_pNormal) {
		m_pNormal = std::make_unique<GLBuffer>();
		m_pNormal->Create(normal);
	} else {
		m_pNormal->BufferSubData(0, normal);
	}
}

void BasicGeometryBuffer::SetColor(const Vector<Vector3>& color)
{
	if (!m_pColor) {
		m_pColor = std::make_unique<GLBuffer>();
		m_pColor->Create(color);
	} else {
		m_pColor->BufferSubData(0, color);
	}
}

void BasicGeometryBuffer::SetTexcoord(const Vector<Vector2>& texcoord)
{
	if (!m_pTexcoord) {
		m_pTexcoord = std::make_unique<GLBuffer>();
		m_pTexcoord->Create(texcoord);
	} else {
		m_pTexcoord->BufferSubData(0, texcoord);
	}
}

void BasicGeometryBuffer::SetIndex(const Vector<int>& index)
{
	if (!m_pIndex) {
		m_pIndex = std::make_unique<GLBuffer>();
		m_pIndex->Create(index);
	} else {
		m_pIndex->BufferSubData(0, index);
	}
}


}
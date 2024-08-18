#ifndef LIGHT_H
#define	LIGHT_H

namespace KI
{
class Light
{
public:
	Light();
	~Light();

	void SetDirection(const Vector3& direction) { m_direction = direction; }
	void SetColor(const Vector3& color) { m_color = color; }

	const Vector3& GetDirection() const { return m_direction; }
	const Vector3& GetColor() const { return m_color; }
private:
	Vector3 m_direction;
	Vector3 m_color;
};

}


#endif // LIGHT_H
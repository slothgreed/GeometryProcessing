#ifndef MOUSE_H
#define MOUSE_H

namespace KI
{

class MouseInput;
class Mouse
{
public:
	Mouse();
	~Mouse();

	const vec2& Position() const { return m_current; };
	const vec2& Delta() const { return m_delta; };
	MY_MOUSE_EVENT Event() const { return m_event; };
	MY_MOUSE_BUTTON Button() const { return m_press; };
	int Wheel() const { return m_wheel; };
	bool Press(MY_MOUSE_BUTTON value) const { return m_press & value; }
	void ApplyMouseInput(const MouseInput& input);
private:

	vec2 m_before;
	vec2 m_current;
	vec2 m_delta;
	bool m_drag;
	MY_MOUSE_EVENT m_event;
	MY_MOUSE_BUTTON m_press;
	int m_wheel;
};
}

#endif MOUSE_H
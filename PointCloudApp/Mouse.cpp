#include "Mouse.h"
#include "MouseInput.h"
namespace KI
{

Mouse::Mouse()
{
}

Mouse::~Mouse()
{
}

void Mouse::ApplyMouseInput(const MouseInput& input)
{
	m_press = input.Press();
	m_event = input.Event();
	m_wheel = input.Wheel();

	if (input.Event() == MY_MOUSE_EVENT::MOUSE_EVENT_DOWN)
	{
		m_before = input.Position();
		m_current = input.Position();
		m_drag = true;
	}
	else if (input.Event() == MY_MOUSE_EVENT::MOUSE_EVENT_MOVE)
	{
		m_before = m_current;
		m_current = input.Position();
		m_drag = false;
	}
	else if (input.Event() == MY_MOUSE_EVENT::MOUSE_EVENT_UP)
	{
		m_before.x = 0;
		m_before.y = 0;
		m_current.x = 0;
		m_current.y = 0;
	}

	m_delta = m_current - m_before;
}
}

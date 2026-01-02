#ifndef MOUSE_CONTROLLER_H
#define MOUSE_CONTROLLER_H

#include "Mouse.h"
#include "Camera.h"
namespace KI
{

class EditContext
{
public:
	EditContext(const Mouse* mouse, const Camera* pCamera)
		: m_pMouse(mouse)
		, m_pCamera(pCamera)
	{
	}
	~EditContext() {};

	const Mouse* GetMouse() const { return m_pMouse; }
	const Camera* GetCamera() const { return m_pCamera; }
private:
	const Mouse* m_pMouse;
	const Camera* m_pCamera;
};


class MouseController
{
public:
	MouseController() {};
	virtual ~MouseController() {}
	virtual bool Move(const EditContext& edit) { return false; };
	virtual bool Wheel(const EditContext& edit) { return false; };

};
}
#endif MOUSE_CONTROLLER_H
#include "pch.h"
#include "Utility.h"

namespace KI
{

void getError()
{
#ifdef _DEBUG
	GLenum error = glGetError();
	if (error != 0) 
	{
		Assert::GLFailed();
	}
#endif
}

}

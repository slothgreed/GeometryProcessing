#include "pch.h"
namespace KI
{

void getError()
{
#ifdef _DEBUG
	GLenum error = glGetError();
	if (error != 0) 
	{
		assert(0);
	}
#endif
}

}

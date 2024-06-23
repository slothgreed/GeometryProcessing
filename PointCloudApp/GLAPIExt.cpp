#include "GLAPIExt.h"
#include <Windows.h>

bool HasExtension(const char* name)
{
	GLint numExtensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	for (int i = 0; i < numExtensions; i++) {
		const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if (strcmp(name, ext) == 0) {
			return true;
		}
	}

	return false;
}

void* GetGLProcAddress(HINSTANCE instance, const char* name)
{
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
		p = (void*)GetProcAddress(instance, name);
	}

	return p;
}



PFNGLDRAWMESHTASKSNVPROC _glDrawMeshTasksNV;
PFNGLDRAWMESHTASKSINDIRECTNVPROC _glDrawMeshTasksIndirectNV;
PFNGLMULTIDRAWMESHTASKSINDIRECTNVPROC _glMultiDrawMeshTasksIndirectNV;
PFNGLMULTIDRAWMESHTASKSINDIRECTCOUNTNVPROC _glMultiDrawMeshTasksIndirectCountNV;

bool LoadGL_NV_mesh_shader(HINSTANCE instance)
{
	if (!HasExtension("GL_NV_mesh_shader")) {
		return false;
	}

	
	_glDrawMeshTasksNV = (PFNGLDRAWMESHTASKSNVPROC)GetGLProcAddress(instance, "glDrawMeshTasksNV");
	_glDrawMeshTasksIndirectNV = (PFNGLDRAWMESHTASKSINDIRECTNVPROC)GetGLProcAddress(instance, "glDrawMeshTasksIndirectNV");
	_glMultiDrawMeshTasksIndirectNV = (PFNGLMULTIDRAWMESHTASKSINDIRECTNVPROC)GetGLProcAddress(instance, "glMultiDrawMeshTasksIndirectNV");
	_glMultiDrawMeshTasksIndirectCountNV = (PFNGLMULTIDRAWMESHTASKSINDIRECTCOUNTNVPROC)GetGLProcAddress(instance, "glMultiDrawMeshTasksIndirectCountNV");

	return
		glDrawMeshTasksNV &&
		glDrawMeshTasksIndirectNV &&
		glMultiDrawMeshTasksIndirectNV &&
		glMultiDrawMeshTasksIndirectCountNV;
}

bool GLAPIExt::Initialize()
{
	HINSTANCE instance = LoadLibraryA("opengl32.dll");
	if (instance == 0) { return false; }
	if (!LoadGL_NV_mesh_shader(instance)) { return false; }
	return true;
}

void glDrawMeshTasksNV(GLuint first, GLuint count)
{
	_glDrawMeshTasksNV(first, count);
}

void glDrawMeshTasksIndirectNV(GLintptr indirect)
{
	_glDrawMeshTasksIndirectNV(indirect);
}

void glMultiDrawMeshTasksIndirectNV(GLintptr indirect, GLsizei drawcount, GLsizei stride)
{
	_glMultiDrawMeshTasksIndirectNV(indirect, drawcount, stride);
}

void glMultiDrawMeshTasksIndirectCountNV(GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
	_glMultiDrawMeshTasksIndirectCountNV(indirect, drawcount, maxdrawcount, stride);
}

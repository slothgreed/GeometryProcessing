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


const glm::ivec3& GLAPIExt::GetMaxComputeWorkGroupCount()
{
	if (m_maxComputeWorkGroupCount.x == 0) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &m_maxComputeWorkGroupCount.x);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &m_maxComputeWorkGroupCount.y);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &m_maxComputeWorkGroupCount.z);
	}

	return m_maxComputeWorkGroupCount;
}

const glm::ivec4& GLAPIExt::GetMaxComputeLocalSize()
{
	if (m_maxComputeLocalSize.x == 0) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &m_maxComputeLocalSize.x);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &m_maxComputeLocalSize.y);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &m_maxComputeLocalSize.z);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &m_maxComputeLocalSize.w);
	}

	GetMeshletMaxVertex();
	GetMeshletMaxPrimitive();

	return m_maxComputeLocalSize;

}

int GLAPIExt::GetMeshletMaxVertex()
{
	if (m_meshletMaxVertex == 0) {
		glGetIntegerv(GL_MAX_MESH_OUTPUT_VERTICES_NV, &m_meshletMaxVertex);
	}

	return m_meshletMaxVertex;
}

int GLAPIExt::GetMeshletMaxPrimitive()
{
	if (m_meshletMaxPrimitive == 0) {
		glGetIntegerv(GL_MAX_MESH_OUTPUT_PRIMITIVES_NV, &m_meshletMaxPrimitive);
	}
	
	return m_meshletMaxPrimitive;
}

GLAPIExt* g_instance = nullptr;


GLAPIExt* GLAPIExt::Info()
{
	if (g_instance == nullptr) {
		g_instance = new GLAPIExt();
	}

	return g_instance;

}

void GLAPIExt::Finalize()
{
	if (g_instance != nullptr) {
		delete g_instance;
		g_instance = nullptr;
	}
}
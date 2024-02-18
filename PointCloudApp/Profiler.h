#ifndef PROFILE_H
#define PROFILE_H
#include <windows.h>

namespace KI
{
class CPUProfiler
{
public:
	CPUProfiler();
	~CPUProfiler();

	void Start();
	void Stop();
	void Output();
private:
	LARGE_INTEGER m_freq;
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_end;

};

class GPUProfiler
{
public:
	GPUProfiler(const String& name);
	~GPUProfiler();

	void Start();
	void Stop();
private:
	String m_name;
	GLuint m_handle;
};

}

#endif PROFILE_H
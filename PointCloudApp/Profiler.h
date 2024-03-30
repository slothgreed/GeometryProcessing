#ifndef PROFILE_H
#define PROFILE_H
#include <windows.h>
#include <chrono>

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

class Timer
{
public:
	Timer() :m_diff(0.0f) {}
	~Timer() {}

	void Begin();
	float End();
private:
	std::chrono::steady_clock::time_point m_begin;
	float m_diff;
};

}

#endif PROFILE_H
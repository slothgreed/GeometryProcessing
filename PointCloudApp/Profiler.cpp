#include "Profiler.h"

Profile::Profile()
{
	QueryPerformanceFrequency(&m_freq);
}

Profile::~Profile()
{
}

void Profile::Start()
{
	QueryPerformanceCounter(&m_start);
}

void Profile::Stop()
{
	QueryPerformanceCounter(&m_end);
}

void Profile::Output()
{
	double time = static_cast<double>(m_end.QuadPart - m_start.QuadPart) * 1000.0 / m_freq.QuadPart;
	printf("time %lf[ms]\n", time);
}

#include "Profiler.h"
namespace KI
{
CPUProfiler::CPUProfiler()
{
	QueryPerformanceFrequency(&m_freq);
}

CPUProfiler::~CPUProfiler()
{
}

void CPUProfiler::Start()
{
	QueryPerformanceCounter(&m_start);
}

void CPUProfiler::Stop()
{
	QueryPerformanceCounter(&m_end);
}

void CPUProfiler::Output()
{
	printf("time %lf[ms]\n", GetMilli());
}
float CPUProfiler::GetFPS()
{
	return 1000.0f / GetMilli();
}

float CPUProfiler::GetMilli()
{
	return static_cast<double>(m_end.QuadPart - m_start.QuadPart) * 1000.0 / m_freq.QuadPart;
}

GPUProfiler::GPUProfiler(const String& name)
{
	glGenQueries(1, &m_handle);
}

GPUProfiler::~GPUProfiler()
{
	glGenQueries(1, &m_handle);
}

void GPUProfiler::Start()
{
	glBeginQuery(GL_TIME_ELAPSED, m_handle);
}

void GPUProfiler::Stop()
{
	glEndQuery(GL_TIME_ELAPSED);

	GLuint qret = false;
	while (!qret) {
		glGetQueryObjectuiv(m_handle, GL_QUERY_RESULT_AVAILABLE, &qret);
	}

	GLuint64EXT nanoTime = 0;
	glGetQueryObjectui64vEXT(m_handle, GL_QUERY_RESULT, &nanoTime);

	m_fps = nanoTime / 1000000;
	//printf(m_name.data());
	//printf("%lf mili second \n", (double)nanoTime / 1000000);
}

float GPUProfiler::GetFPS()
{
	return m_fps;
}


void Timer::Start()
{
	m_begin = std::chrono::high_resolution_clock::now();
}

float Timer::Current()
{
	return Stop();
}
float Timer::Stop()
{
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double, std::milli>(end - m_begin).count() / 1000;
}

}

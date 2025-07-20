#include "Profiler.h"
#include <nvml.h>
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

float CPUProfiler::GetUsage()
{
	static FILETIME prevIdleTime, prevKernelTime, prevUserTime;
	FILETIME idleTime, kernelTime, userTime;

	if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return 0.0f;

	ULONGLONG idle = *((ULONGLONG*)&idleTime) - *((ULONGLONG*)&prevIdleTime);
	ULONGLONG kernel = *((ULONGLONG*)&kernelTime) - *((ULONGLONG*)&prevKernelTime);
	ULONGLONG user = *((ULONGLONG*)&userTime) - *((ULONGLONG*)&prevUserTime);

	ULONGLONG total = kernel + user;

	prevIdleTime = idleTime;
	prevKernelTime = kernelTime;
	prevUserTime = userTime;

	return total ? (100.0f * (total - idle) / total) : 0.0f;
}
GPUProfiler::GPUProfiler(const String& name)
{
	glGenQueries(1, &m_handle);

	nvmlInit();
}

GPUProfiler::~GPUProfiler()
{
	glGenQueries(1, &m_handle);
	nvmlShutdown();
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

float GPUProfiler::GetUsage()
{
	nvmlDevice_t device;
	nvmlDeviceGetHandleByIndex(0, &device);

	nvmlUtilization_t utilization;
	nvmlDeviceGetUtilizationRates(device, &utilization);

	return utilization.gpu; // 使用率（%）
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

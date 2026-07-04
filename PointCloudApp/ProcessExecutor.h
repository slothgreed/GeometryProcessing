#ifndef PROCESS_EXECUTOR_H
#define PROCESS_EXECUTOR_H
#include <Windows.h>

namespace KI
{

class ProcessExecutor
{
public:
	ProcessExecutor(const String& pName):processName(pName) {};
	~ProcessExecutor() { if (m_async)FinalizeASync(); };
	bool ExecuteSync() { return ExecuteSync(""); }
	bool ExecuteSync(const String& commandArgs);
	bool ExecuteASync() { return ExecuteASync(""); }
	bool ExecuteASync(const String& commandArgs);
	bool FinalizeASync();
private:
	bool m_async = false;
	PROCESS_INFORMATION m_processInfo{};
	String processName;
};

}
#endif PROCESS_EXECUTOR_H
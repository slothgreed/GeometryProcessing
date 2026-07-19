#ifndef PROCESS_EXECUTOR_H
#define PROCESS_EXECUTOR_H
#include <Windows.h>

namespace KI
{

class ClientPipe
{
public:
	ClientPipe() :m_hPipe(INVALID_HANDLE_VALUE) {};
	~ClientPipe() {};

	bool Open();
	void SendCommand(const std::string& message);
	void Close();
private:
	HANDLE m_hPipe;

};
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
	void SendCommand(const std::string& message) { m_pipe.SendCommand(message); }
private:
	bool m_async = false;
	PROCESS_INFORMATION m_processInfo{};
	String processName;
	ClientPipe m_pipe;
};

class AIProcessor : public ProcessExecutor
{
	AIProcessor(const String& pName) :ProcessExecutor(pName) {};
public:
	static AIProcessor& Instance()
	{
		static AIProcessor instance = AIProcessor("GeometryAI.exe");
		return instance;
	}

	static void Dispose()
	{
		Instance().FinalizeASync();
	}
};
}

#endif PROCESS_EXECUTOR_H
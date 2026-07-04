#include "ProcessExecutor.h"
namespace KI
{

bool ProcessExecutor::ExecuteSync(const String& commandArgs)
{
	std::cout << "Executing process: " << processName << std::endl;
    STARTUPINFO si{};
    auto args = processName + " " + commandArgs;
    BOOL result = CreateProcess(
        nullptr,        // 実行ファイル
        args.data(),    // コマンドライン
		nullptr, nullptr,
		FALSE,
		0,
		nullptr, nullptr,
		&si,
        &m_processInfo);

    if (!result) {
		std::cout << "CreateProcess failed : " << GetLastError() << std::endl;
        return (bool)result;
    }

    std::cout << "AI Process Started" << std::endl;

    // AI終了待ち
    WaitForSingleObject(m_processInfo.hProcess, INFINITE);

    CloseHandle(m_processInfo.hThread);
    CloseHandle(m_processInfo.hProcess);
    return true;
}

bool ProcessExecutor::ExecuteASync(const String& commandArgs)
{
    std::cout << "Executing process: " << processName << std::endl;
    STARTUPINFO si{};
    auto args = processName + " " + commandArgs;
    BOOL result = CreateProcess(
        nullptr,        // 実行ファイル
        args.data(),    // コマンドライン
        nullptr, nullptr,
        FALSE,
        0,
        nullptr, nullptr,
        &si,
        &m_processInfo);

    if (!result) {
        std::cout << "CreateProcess failed : " << GetLastError() << std::endl;
        return (bool)result;
    }

    m_async = true;
    std::cout << "AI Process Started" << std::endl;


    return true;
}

bool ProcessExecutor::FinalizeASync()
{
    // AI終了待ち
    WaitForSingleObject(m_processInfo.hProcess, INFINITE);

    CloseHandle(m_processInfo.hThread);
    CloseHandle(m_processInfo.hProcess);
    m_async = false;
    return true;
}
}
#include "ProcessExecutor.h"
#include "Utility.h"
namespace KI
{

bool ProcessExecutor::ExecuteSync(const String& commandArgs)
{
	std::cout << "Executing process: " << processName << std::endl;
    STARTUPINFO si{};
    si.cb = sizeof(si);
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
    si.cb = sizeof(si);
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

    if (!m_pipe.Open()) {
        std::cerr << "GeometryAI pipe connection timed out." << std::endl;
        TerminateProcess(m_processInfo.hProcess, 1);
        WaitForSingleObject(m_processInfo.hProcess, INFINITE);
        CloseHandle(m_processInfo.hThread);
        CloseHandle(m_processInfo.hProcess);
        m_processInfo = {};
        m_async = false;
        return false;
    }

    return true;
}

bool ProcessExecutor::FinalizeASync()
{
    m_pipe.Close();
	if (m_async == false) return true;
    // AI終了待ち
    WaitForSingleObject(m_processInfo.hProcess, INFINITE);

    CloseHandle(m_processInfo.hThread);
    CloseHandle(m_processInfo.hProcess);
    m_async = false;
    return true;
}


bool ClientPipe::Open()
{
    if (m_hPipe != INVALID_HANDLE_VALUE) {
        return true;
    }

    constexpr ULONGLONG timeoutMilliseconds = 30000;
    const ULONGLONG startTime = GetTickCount64();
    DWORD lastError = ERROR_SUCCESS;

    while (GetTickCount64() - startTime < timeoutMilliseconds) {
        m_hPipe = CreateFileW(
            LR"(\\.\pipe\GeometryAIPipe)",
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);
        if (m_hPipe != INVALID_HANDLE_VALUE) {
            std::cout << "GeometryAI pipe connected." << std::endl;
            return true;
        }

        lastError = GetLastError();
        if (lastError == ERROR_PIPE_BUSY) {
            WaitNamedPipeW(LR"(\\.\pipe\GeometryAIPipe)", 100);
        } else {
            Sleep(100);
        }
    }

    std::cerr << "GeometryAI pipe open failed : " << lastError << std::endl;
    return false;
}

void ClientPipe::SendCommand(const std::string& message)
{
    if (m_hPipe == INVALID_HANDLE_VALUE && !Open()) {
        std::cerr << "Pipe is not open." << std::endl;
        return;
    }
    DWORD bytesWritten = 0;
    if (!WriteFile(
        m_hPipe,
        message.data(),
        static_cast<DWORD>(message.size()),
        &bytesWritten,
        nullptr)) {
        std::cerr << "Pipe write failed : " << GetLastError() << std::endl;
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
        return;
    }

    char buffer[256];
    DWORD bytesRead = 0;

    if (!ReadFile(
        m_hPipe,
        buffer,
        sizeof(buffer) - 1,
        &bytesRead,
        nullptr)) {
        std::cerr << "Pipe read failed : " << GetLastError() << std::endl;
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
        return;
    }

    buffer[bytesRead] = '\0';

    std::cout << "Response : " << buffer << std::endl;
}

void ClientPipe::Close()
{
    if (m_hPipe == INVALID_HANDLE_VALUE) {
        return;
    }

    SendCommand("--exit");
    if (m_hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hPipe);
    }
    m_hPipe = INVALID_HANDLE_VALUE;
}
}

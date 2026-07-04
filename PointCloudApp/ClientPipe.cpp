#include "ClientPipe.h"
#include "ProcessExecutor.h"
#include "Utility.h"
namespace KI
{
	
bool ClientPipe::Open()
{
    for(int i = 0; i < 10; i++) {
        m_hPipe = CreateFileW(
            LR"(\\.\pipe\GeometryAIPipe)",
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);
        if(m_hPipe != INVALID_HANDLE_VALUE) {
            break;
		}
        Sleep(100);
    }

    return m_hPipe != INVALID_HANDLE_VALUE;
}

void ClientPipe::SendCommand(const std::string& message)
{
    if(m_hPipe == INVALID_HANDLE_VALUE) {
		Assert::Failed();
        std::cerr << "Pipe is not open." << std::endl;
        return;
	}
    DWORD bytesWritten = 0;
    WriteFile(
        m_hPipe,
        message.data(),
        static_cast<DWORD>(message.size()),
        &bytesWritten,
        nullptr);

    char buffer[256];
    DWORD bytesRead = 0;

    ReadFile(
        m_hPipe,
        buffer,
        sizeof(buffer) - 1,
        &bytesRead,
        nullptr);

    buffer[bytesRead] = '\0';

    std::cout << "Response : " << buffer << std::endl;
}

void ClientPipe::Close()
{
    SendCommand("--exit");
    CloseHandle(m_hPipe);
	m_hPipe = INVALID_HANDLE_VALUE;
}
}
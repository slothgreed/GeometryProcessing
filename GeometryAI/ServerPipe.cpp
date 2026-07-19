#include "ServerPipe.h"
#include <iostream>
namespace KI
{
void ServerPipe::Connect()
{
    const wchar_t* pipeName = LR"(\\.\pipe\GeometryAIPipe)";

    m_hPipe = CreateNamedPipeW(
        pipeName,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE |
        PIPE_READMODE_MESSAGE |
        PIPE_WAIT,
        1,                  // 最大接続数
        4096,               // 送信バッファ
        4096,               // 受信バッファ
        0,
        nullptr);

    if (m_hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateNamedPipe failed." << std::endl;
        return;
    }

    std::cout << "Waiting for client..." << std::endl;

    BOOL connected = ConnectNamedPipe(m_hPipe, nullptr);

    // クライアントが既に接続済みの場合
    if (!connected && GetLastError() == ERROR_PIPE_CONNECTED) {
        connected = TRUE;
    }

    if (!connected) {
        std::cerr << "ConnectNamedPipe failed." << std::endl;
        CloseHandle(m_hPipe);
        return;
    }

    std::cout << "Client connected." << std::endl;
    for (;;) {
        if (!ProcessLoop())
            break;
    }

    Close();
}

bool ServerPipe::ProcessLoop()
{
    char buffer[1024];
    DWORD bytesRead = 0;

    if (!ReadFile(
        m_hPipe,
        buffer,
        sizeof(buffer) - sizeof(wchar_t),
        &bytesRead,
        nullptr)) {
        return false;
    }

    buffer[bytesRead] = '\0';

    std::cout << "Received : " << buffer << std::endl;

    if (std::string_view(buffer) == "--exit") {
        return false;
    }
    for(auto& receiver : m_receiver) {
        if (receiver.second->Execute(std::string(buffer))) {
            std::cout << "Command processed by: " << receiver.first << std::endl;
            break;
        }
	}
    
    std::string response = "OK";
    DWORD bytesWritten = 0;
    WriteFile(
        m_hPipe,
        response.c_str(),
        static_cast<DWORD>(response.size()),
        &bytesWritten,
        nullptr);

    return true;
}
void ServerPipe::Close()
{
    DisconnectNamedPipe(m_hPipe);
    CloseHandle(m_hPipe);

    std::cout << "Disconnected." << std::endl;
}

}
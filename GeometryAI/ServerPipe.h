#ifndef SERVER_PIPE_H
#define SERVER_PIPE_H
#include <Windows.h>
#include <unordered_map>
#include <memory>
#include "ICommandReceiver.h"
namespace KI
{
class ICommandReceiver;
class ServerPipe
{
public:
	ServerPipe():m_hPipe(nullptr) {};
	~ServerPipe() {};

	void SetReceiveCommand(const std::shared_ptr<ICommandReceiver>& pReceiver) { m_receiver[pReceiver->GetCommandName()] = pReceiver; }

    void Connect();
private:
	void Close();
    bool ProcessLoop();
    HANDLE m_hPipe;
	std::unordered_map<std::string, std::shared_ptr<ICommandReceiver>> m_receiver;
};

}
#endif SERVER_PIPE_H
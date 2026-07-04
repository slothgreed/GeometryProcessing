#ifndef SERVER_PIPE_H
#define SERVER_PIPE_H
#include <Windows.h>
namespace KI
{
class ServerPipe
{
public:
	ServerPipe():m_hPipe(nullptr) {};
	~ServerPipe() {};

    void Connect();
private:
	void Close();
    bool ProcessLoop();
    HANDLE m_hPipe;
};

}
#endif SERVER_PIPE_H
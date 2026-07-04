#ifndef CLIENT_PIPE_H
#define CLIENT_PIPE_H
#include <Windows.h>
namespace KI
{
class ClientPipe
{
public:
	ClientPipe():m_hPipe(INVALID_HANDLE_VALUE){};
	~ClientPipe() {};

	bool Open();
	void SendCommand(const std::string& message);
	void Close();
private:
	HANDLE m_hPipe;

};

}

#endif CLIENT_PIPE_H
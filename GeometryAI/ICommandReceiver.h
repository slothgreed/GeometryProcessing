#ifndef I_COMMAND_RECEIVER_H
#define I_COMMAND_RECEIVER_H
#include <string>
namespace KI
{
class ICommandReceiver
{
public:
	ICommandReceiver() {};
	~ICommandReceiver() {};
	virtual std::string GetCommandName() const = 0;
	virtual bool Execute(const std::string& command) = 0;

private:

};

}

#endif I_COMMAND_RECEIVER_H
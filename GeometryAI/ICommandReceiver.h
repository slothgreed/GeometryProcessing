#ifndef I_COMMAND_RECEIVER_H
#define I_COMMAND_RECEIVER_H
#include <string>
#include <vector>
namespace KI
{
class ICommandReceiver
{
public:
	ICommandReceiver() {};
	~ICommandReceiver() {};
	virtual std::string GetCommandName() const = 0;
	virtual bool Execute(const std::string& command) = 0;

	std::vector<std::string> Parse(const std::string& command);
private:

};

}

#endif I_COMMAND_RECEIVER_H
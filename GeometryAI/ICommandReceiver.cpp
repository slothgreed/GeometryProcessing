#include "ICommandReceiver.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace KI
{

std::vector<std::string> ICommandReceiver::Parse(const std::string& command)
{
    std::vector<std::string> tokens;

    std::stringstream ss(command);
    std::string token;

    while (ss >> token) {
        tokens.push_back(std::move(token));
    }

    for(size_t i = 0; i < tokens.size(); ++i) {
        std::cout << "Token[" << i << "]: " << tokens[i] << std::endl;
	}
    return tokens;
}

}
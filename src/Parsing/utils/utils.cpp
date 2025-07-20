#include "../../../inc/webserv.hpp"

void printMap(const std::map<std::string, std::string>& m)
{
    for (std::map<std::string, std::string>::const_iterator it = m.begin(); it != m.end(); ++it)
    {
        std::cout << it->first << GREEN " equal to " RESET << it->second << std::endl;
    }
}

void logError(int status, const std::string& error_message)
{
    std::cerr << RED "Error " << status << ": " << error_message << RESET << std::endl;
}
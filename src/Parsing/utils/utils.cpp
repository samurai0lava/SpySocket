#include "../../../inc/webserv.hpp"

void printMap(const std::map<std::string, std::string>& m)
{
    for (std::map<std::string, std::string>::const_iterator it = m.begin(); it != m.end(); ++it)
    {
        std::cout << it->first << " : " << it->second << std::endl;
    }
}
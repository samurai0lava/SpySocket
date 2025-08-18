#include "../inc/webserv.hpp"

void handleMethod(int client_fd,ParsingRequest* parser, const ConfigStruct& config,Servers &serv)
{
    std::string method = parser->getStartLine()["method"];
    if(method == "GET")
    {
        std::cout<<"111111111 "<<std::endl;
        std::string uri = parser->getStartLine()["uri"];
        ConfigStruct &mutableConfig = const_cast<ConfigStruct&>(config);
        Get MGet(client_fd,parser, mutableConfig, serv, uri);
        MGet.MethodGet();
        std::cout<<"8888888888888888888888\n";
    }
    else if (method == "DELETE")
    {
        std::cout<<"DELETE WE BE HERE "<<std::endl;
        std::string uri = parser->getStartLine()["uri"];
        ConfigStruct &mutableConfig = const_cast<ConfigStruct&>(config);
        DeleteMethode MDelete;
        MDelete.PerformDelete(client_fd, uri, mutableConfig);
    }
}
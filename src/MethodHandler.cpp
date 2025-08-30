#include "../inc/webserv.hpp"
// #include "../inc/CClient.h"

void handleMethod(int client_fd,ParsingRequest* parser, const ConfigStruct& config,Servers &serv,CClient &client_data)
{

    std::string method = parser->getStartLine()["method"];
    std::string uri = parser->getStartLine()["uri"];
    // std::cout<<" "
    ConfigStruct &mutableConfig = const_cast<ConfigStruct&>(config);
    client_data = CClient(method, uri,client_fd, mutableConfig, serv, parser);
    client_data.SendHeader = false;
    // client_data.printInfo();

    // if(method == "GET")
    // {

    //     return ;
    // }
    // else if (method == "DELETE")
    // {
        // DeleteMethode MDelete;
        // MDelete.PerformDelete(client_fd, uri, mutableConfig);
    // }
    // else 
    // if(method == "POST")
    // {
    //     return postMethod(uri, mutableConfig, *parser);
    // }
    return ;
}
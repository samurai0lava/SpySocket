#include "../inc/webserv.hpp"
// #include "../inc/CClient.h"

string handleMethod(int client_fd,ParsingRequest* parser, const ConfigStruct& config,Servers &serv,CClient &client_data)
{

    std::string method = parser->getStartLine()["method"];
    std::string uri = parser->getStartLine()["uri"];
    // std::cout<<" "
    ConfigStruct &mutableConfig = const_cast<ConfigStruct&>(config);
    client_data = CClient(method, uri,client_fd, mutableConfig, serv, parser);

    if(method == "GET")
    {
        Get MGet(client_fd,parser, mutableConfig, serv, uri);
        return( MGet.MethodGet() );
    }
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
    return "";
}
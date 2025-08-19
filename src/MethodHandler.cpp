#include "../inc/webserv.hpp"

string handleMethod(int client_fd,ParsingRequest* parser, const ConfigStruct& config,Servers &serv)
{
    
    std::string method = parser->getStartLine()["method"];
    std::string uri = parser->getStartLine()["uri"];
    ConfigStruct &mutableConfig = const_cast<ConfigStruct&>(config);

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
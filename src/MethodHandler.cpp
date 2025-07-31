#include "../inc/MethodHandler.hpp"

void handleMethod(ParsingRequest* parser, const ConfigStruct& config,Servers &serv)
{
    std::string method = parser->getStartLine()["method"];
    if(method == "GET")
    {
        std::string uri = parser->getStartLine()["uri"];
        ConfigStruct &mutableConfig = const_cast<ConfigStruct&>(config);
        Get MGet(parser, mutableConfig, serv, uri);
        MGet.MethodGet();
    }
}
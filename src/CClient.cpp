#include "../inc/CClient.hpp"
// #include "CClient.hpp"

CClient::CClient()
{
    
}

CClient::CClient(string NameMethod, string uri,int FdClient, ConfigStruct MConfig, Servers serv, ParsingRequest* parser):
    NameMethod(NameMethod),uri(uri),FdClient(FdClient),mutableConfig(MConfig),serv(serv),parser(parser)

{
}

CClient::~CClient()
{
}

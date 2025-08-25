#include "../inc/CClient.hpp"
// #include "CClient.hpp"
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

void CClient::printInfo() const {
    std::cout << "===== Client Info =====" << std::endl;
    std::cout << "Method: " << NameMethod << std::endl;
    std::cout << "URI: " << uri << std::endl;
    std::cout << "FdClient: " << FdClient << std::endl;
    std::cout << "Read Buffer: " << readbuf << std::endl;
    std::cout << "Offset: " << offset << std::endl;
    std::cout << "Size File: " << sizeFile << std::endl;

    // Optional: print parser address
    std::cout << "Parser ptr: " << parser << std::endl;

    // Config info (if you want details from mutableConfig)
    std::cout << "Config root: " << mutableConfig.root << std::endl;
    std::cout << "Config indexPage: " << mutableConfig.indexPage << std::endl;

    std::cout << "=======================" << std::endl;
}

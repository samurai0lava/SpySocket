#ifndef CCLIENT_HPP
# define CCLIENT_HPP

#include "webserv.hpp"

class CClient
{
    public:
        std::string NameMethod;
        std::string uri;
        int FdClient;
        ConfigStruct mutableConfig ;
        Servers serv;
        ParsingRequest *parser;
        CClient ();
        CClient(string NameMethod,string uri,int FdClient,ConfigStruct MConfig,
            Servers serv,ParsingRequest *parser);
         ~CClient();





};



#endif

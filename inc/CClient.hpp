#ifndef CCLIENT_HPP
# define CCLIENT_HPP

#include "../include/server.hpp"
#include "parsing_request.hpp"
#include "../include/Config.hpp"
#include <string>



class CClient   
{
    public:
        std::string _name_location;
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

        void printInfo() const;



};



#endif

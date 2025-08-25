#ifndef CCLIENT_HPP
#define CCLIENT_HPP

#include <string>
#include <fstream>
#include "webserv.hpp"  // only include general stuff; do NOT include Get.hpp

class CClient
{
    public:
        std::string NameMethod;
        std::string uri;
        int FdClient;
        ConfigStruct mutableConfig;
        Servers serv;
        ParsingRequest *parser;
        std::ifstream file;
        ssize_t readbuf;
        ssize_t offset;
        ssize_t sizeFile;

        CClient();
        CClient(std::string NameMethod, std::string uri, int FdClient,
                ConfigStruct MConfig, Servers serv, ParsingRequest *parser);
        ~CClient();

        void printInfo() const;
};

#endif

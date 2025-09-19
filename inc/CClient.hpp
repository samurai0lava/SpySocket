#ifndef CCLIENT_HPP
# define CCLIENT_HPP

#include "../include/server.hpp"
#include "parsing_request.hpp"
#include "../include/Config.hpp"
#include "RespondError.hpp"
#include <string>


class CClient
{
public:
    std::string _name_location;
    std::string NameMethod;
    std::string uri;
    int FdClient;
    ConfigStruct mutableConfig;
    Servers serv;
    ParsingRequest* parser;
    bool SendHeader;
    bool readyToSendAllResponse;
    bool chunkedSending;
    size_t chunkSize;
    size_t bytesSent;
    std::string response;
    std::string filePath;
    size_t fileSize;
    off_t offset;
    int fileFd;
    bool intialized;
    bool Chunked;
    // bool               
    CClient();
    CClient(string NameMethod, string uri, int FdClient, ConfigStruct MConfig,
        Servers serv, ParsingRequest* parser);
    ~CClient();
    // string setupChunkedSending(const std::string & filePath);
    string HandleAllMethod();
    // std::string GenerateResErr(int errorCode);
    void printInfo() const;



};



#endif

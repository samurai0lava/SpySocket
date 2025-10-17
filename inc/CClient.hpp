#ifndef CCLIENT_HPP
# define CCLIENT_HPP

#include "../include/server.hpp"
#include "parsing_request.hpp"
#include "../include/Config.hpp"
#include "RespondError.hpp"
#include "CGI.hpp"
#include <string>


class CClient : public ParsingRequest
{
public:
    std::string _name_location;
    std::string NameMethod;
    std::string uri;
    int FdClient;
    ConfigStruct mutableConfig;
    Servers *serv;
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
    
    // CGI-related fields
    CGI* cgi_handler;
    bool is_cgi_request;
    bool cgi_headers_sent;
    std::string cgi_body_buffer;
    
    // bool               
    CClient();
    CClient(std::string NameMethod, std::string uri, int FdClient, ConfigStruct MConfig,
        Servers *serv, ParsingRequest* parser);
    ~CClient();
    // std::string setupChunkedSending(const std::std::string & filePath);
    std::string HandleAllMethod();
    std::string HandleCGIMethod();
    std::string formatCGIResponse(const std::string& cgi_output);
    // std::string GenerateResErr(int errorCode);
    void printInfo() const;



};


// curl -v http://localhost:8080 2>&1 | grep -E "(Set-Cookie|Cookie)"
#endif

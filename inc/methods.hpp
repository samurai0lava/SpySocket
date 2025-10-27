#pragma once
#include "server.hpp"
#include <vector>
#include <string>
#include "../inc/webserv.hpp"

class Error
{
public:
    static std::string notFound() {
        return  "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 48\r\n"
                "\r\n"
                "<html><body><h1>404 Not Found</h1></body></html>";
    };
};

class Post : public Error
{
public:
    std::map<std::string, std::string> header;
    std::string path;
    std::string contentType;
    std::string contentLength;
    std::string returnType;
    std::string body;
    Post() : body("") {};
};

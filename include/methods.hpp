#pragma once
#include "server.hpp"
#include <vector>
#include <string>
#include "../inc/webserv.hpp"

class Error
{
public:
    static string notFound() {
        return  "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 48\r\n"
                "\r\n"
                "<html><body><h1>404 Not Found</h1></body></html>";
    };
};

class Post : public Error
{
    // the retrieved data from the request
public:
    map<string, string> header;
    string path;
    string contentType;
    string contentLength;
    string returnType;
    string body;

    Post() : body("") {};
    // void fillImportantFields();
};

string handleMethod(int fd, ParsingRequest* parser, Config *conf);

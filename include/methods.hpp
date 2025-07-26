#pragma once
#include "server.hpp"
#include <vector>
#include <string>

class Post
{
    //the retrieved data from the request
public :
    map<string, string> header;
    string path;
    string contentType;
    string contentLength;
    string returnType;
    string body;

    Post() : body("")
    {};
    // void fillImportantFields();
};
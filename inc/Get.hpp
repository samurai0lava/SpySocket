
#ifndef GET_HPP
# define GET_HPP

#include "webserv.hpp"



class Get 
{
    public :

        int client_fd;
        ParsingRequest * parser;
        ConfigStruct &config;
        std::string _name_location;
        Servers &serv;
        std::string uri;
        Get(int client_fd,ParsingRequest * parser,ConfigStruct &config,Servers &serv,std::string uri);
        ~Get();
        string MethodGet();
        std::string getMimeType(const std::string& path);
        std::string matchLocation(const std::string &requestPath, const ConfigStruct &server);
        std::string generateAutoIndex(const std::string &directoryPath);
        bool isDirectory(const std::string& path);
        bool isFile(const std::string& path);
        bool pathExists(const std::string& path);
 
};
#endif
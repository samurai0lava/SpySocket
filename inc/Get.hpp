#ifndef GET_HPP
# define GET_HPP

#include "webserv.hpp"
#include "CClient.hpp"
class Get
{

<<<<<<< HEAD
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
 
=======
private:
    CClient& client;
public:
    Get();
    Get(CClient& client);
    ~Get();
    void setupFromClient(const CClient& client) {
        this->client.uri = client.uri;
        this->client.mutableConfig = client.mutableConfig;
        //  anything else you need
    }
    std::string getMimeType(const std::string& path);
    std::string matchLocation(const std::string& requestPath, const ConfigStruct& server);
    std::string generateAutoIndex(const std::string& directoryPath);
    bool isDirectory(const std::string& path);
    bool isFile(const std::string& path);
    bool pathExists(const std::string& path);
    std::string buildHttpHeaders(const std::string& path, size_t fileSize);
    std::string pathIsFile(std::string matchLocation);
    std::string handleDirectoryWithIndex(std::string indexPath);
    std::string handleDirectoryWithAutoIndex(std::string matchLocation);
    std::string MethodGet();
    string setupChunkedSending(const std::string& filePath);
    void printLocationStruct(const LocationStruct& loc);
    string buildRedirectResponse(int statusCode, const std::string& target);

>>>>>>> ilyass
};

#endif

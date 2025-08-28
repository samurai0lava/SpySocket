#ifndef GET_HPP
# define GET_HPP

#include "webserv.hpp"
#include "CClient.hpp"
class Get : public CClient
{

    public:
        Get();  
        Get(const CClient& client) : CClient(client) {} ; // copy base part
        ~Get();
        void setupFromClient(const CClient& client) {
        this->uri = client.uri;
        this->mutableConfig = client.mutableConfig;
        // copy anything else you need
    }
        std::string getMimeType(const std::string& path);
        std::string matchLocation(const std::string &requestPath, const ConfigStruct &server);
        std::string generateAutoIndex(const std::string &directoryPath);
        bool isDirectory(const std::string& path);
        bool isFile(const std::string& path);
        bool pathExists(const std::string& path);
        std::string buildHttpHeaders(const std::string& path, size_t fileSize);
        std::string pathIsFile(std::string matchLocation);
        std::string handleDirectoryWithIndex(std::string indexPath);
        std::string handleDirectoryWithAutoIndex(std::string matchLocation);
        std::string MethodGet();
        string setupChunkedSending(const std::string  & filePath);
        void printLocationStruct(const LocationStruct &loc);

};

#endif

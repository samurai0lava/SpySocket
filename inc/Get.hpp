#ifndef GET_HPP
#define GET_HPP

  // full definition required for inheritance
#include "webserv.hpp"  // optional, only include if needed for structs like Servers/ParsingRequest
#include "CClient.hpp"
class Get : public CClient
{
    public:
        std::string _name_location;

        Get(int client_fd, ParsingRequest *parser,
            ConfigStruct &config, Servers &serv, std::string uri);
        ~Get();

        std::string getMimeType(const std::string& path);
        std::string matchLocation(const std::string &requestPath, const ConfigStruct &server);
        std::string generateAutoIndex(const std::string& directoryPath);
        bool isDirectory(const std::string& path);
        bool isFile(const std::string& path);
        bool pathExists(const std::string& path);
        std::string buildHttpHeaders(const std::string& path, size_t fileSize);
        std::string pathIsFile(std::string matchLocation);
        std::string handleDirectoryWithIndex(std::string indexPath);
        std::string handleDirectoryWithAutoIndex(std::string matchLocation);
        std::string MethodGet();
};

#endif

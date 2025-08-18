#ifndef STRUCT_HPP
#define STRUCT_HPP
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <cstring> 
#include <vector>
#define WHITESPACE "\n\r\t\f\v "
#define DECIMAL "0123456789"
#define CHUNK_SIZE  16384

enum
{
	listen_,
	root,
	server_name,
	index_page,
	location,
	error_page,
    autoindex,
	host,
    client_max_body_size,
    cgi_bin,
	not_found
};

enum
{
	location_root,
	location_auto_index,
    method,
	location_index,
    _return,
    cgi_path,
	cgi_ext,
	upload_enabled,
	upload_path
};
struct LocationStruct
{
	bool autoIndex;
	std::set<std::string> allowedMethods;
	std::string root;
	std::string indexPage;
    // std::string _return;
	std::vector<std::pair<std::string,std::string> > _return;
	std::vector<std::string> cgi_path;
	std::vector<std::string> cgi_ext;
	bool upload_enabled;
	std::string upload_path;
	// std::vector<std::pair<std::string,std::string> > cgi_path;
	// std::vector<std::pair<std::string,std::string> > cgi_ext;
};
struct ClientSendState {
	 int clientFd;              // Client socket file descriptor
    std::string filePath;      // Full path of the file being sent
    int fileFd;                // File descriptor for the file
    off_t offset;              // Current offset in the file
    size_t fileSize;           // Total size of the file
    bool sendingHeaders;       // True if headers still need to be sent
    std::string headers;       // HTTP headers to send
    char buffer[CHUNK_SIZE];   // Chunk buffer for reading file
	size_t bufferLen;          // How many bytes currently in buffer
    size_t bufferSent;         // How many bytes already sent from buffer

     ClientSendState()
        : clientFd(-1), fileFd(-1), offset(0),
          fileSize(0), sendingHeaders(true),
          bufferLen(0), bufferSent(0)
    {
        memset(buffer, 0, CHUNK_SIZE);
    }
};
struct ConfigStruct
{
    std::string								serverName;
	std::string								host;
	std::string								root;
    std::string								indexPage;
	bool									autoIndex;
	size_t									clientMaxBodySize;
	std::vector<unsigned short> listen;
	// std::vector<std::pair<std::string, unsigned short> > listen;
	std::vector<std::pair<std::string,LocationStruct> > location;
	std::vector<std::pair<std::string,std::string> > errorPage;
};






















#endif
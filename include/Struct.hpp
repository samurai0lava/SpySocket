#ifndef STRUCT_HPP
#define STRUCT_HPP
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <vector>
#define WHITESPACE "\n\r\t\f\v "
#define DECIMAL "0123456789"
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
	cgi_ext
};
struct LocationStruct
{
	bool autoIndex;
	std::set<std::string> allowedMethods;
	std::string root;
	std::string indexPage;
    std::string _return;
	std::vector<std::string> cgi_path;
	std::vector<std::string> cgi_ext;	
	// std::vector<std::pair<std::string,std::string> > cgi_path;
	// std::vector<std::pair<std::string,std::string> > cgi_ext;
};

struct ConfigStruct
{
    std::string								serverName;
	std::string								host;
	std::string								root;
    std::string								indexPage;
	bool									autoIndex;
	size_t									clientMaxBodySize;
	int listen;
	// std::vector<std::pair<std::string, unsigned short> > listen;
	std::vector<std::pair<std::string,LocationStruct> > location;
	std::vector<std::pair<std::string,std::string> > errorPage;
};

#endif
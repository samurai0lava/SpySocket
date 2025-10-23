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
	location,
	error_page,
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
	std::vector<std::pair<std::string, std::string> > _return;
	std::vector<std::string> cgi_path;
	std::vector<std::string> cgi_ext;
	bool upload_enabled;
	std::string upload_path;
	std::map<std::string, std::string> url_encoded; //heeepa
	// std::vector<std::pair<std::string,std::string> > cgi_path;
	// std::vector<std::pair<std::string,std::string> > cgi_ext;
};

// config->_cluster

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
	std::vector<std::pair<std::string, LocationStruct> > location; //switch to map maybe?
	std::vector<std::pair<std::string, std::string> > errorPage;
};






















#endif
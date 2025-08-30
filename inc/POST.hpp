#pragma once

#include "webserv.hpp"
#include <cmath>

string	postMethod(string uri, ConfigStruct config, ParsingRequest& parser);

std::pair<std::string, LocationStruct> get_location(const std::string& requestPath, const ConfigStruct& server);

string unchunk_content(char *buffer);

string unchunk_data(char *chunk, size_t chunk_size);


std::vector<std::string> split(std::string s, std::string delimiters);
std::string generate_filename(string type, string termination);


// string	refactor_data(const char *buffer, size_t buffer_size);

void	refactor_data(string& buffer, const char* data, size_t len);


// class POST
// {
// protected:
//     map<string, string> url_encode;
// public:
//     // void fill_map(string key, string value)
//     // {
        
//     // };
//     // map<string, string> get_map(string key, string value);
// };
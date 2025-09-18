#pragma once

#include "webserv.hpp"
#include <cmath>

string	postMethod(string uri, ConfigStruct config, ParsingRequest &parser);

std::pair<std::string,
	LocationStruct> get_location(const std::string &requestPath,
	const ConfigStruct &server);

string	unchunk_content(char *buffer);

string	unchunk_data(char *chunk, size_t chunk_size);

std::vector<std::string> split(std::string s, std::string delimiters);
std::string generate_filename(string type, string termination);

void	reset_refactor_data_state(void);
bool	is_chunked_transfer_complete(void);
bool	refactor_data(string &buffer, const char *data, size_t len);

// messages
string	handle_redirect(std::pair<std::string, LocationStruct> location);
string	handle_notAllowed(std::pair<std::string, LocationStruct> location);
string	notFound(void);
string	bad_request(void);
string	forbidden_403(void);
string	internal_error(void);
string	created_success(void);
std::string OK_200(std::string &body);

#pragma once

#include "webserv.hpp"
#include <cmath>

std::string	postMethod(std::string uri, ConfigStruct config, ParsingRequest &parser);

std::pair<std::string,
	LocationStruct> get_location(const std::string &requestPath,
	const ConfigStruct &server);

std::string	unchunk_content(char *buffer);

std::string	unchunk_data(char *chunk, size_t chunk_size);

std::vector<std::string> split(std::string s, std::string delimiters);
std::string generate_filename(std::string type, std::string termination);

void	reset_refactor_data_state(void);
bool	is_chunked_transfer_complete(void);
bool	refactor_data(std::string &buffer, const char *data, size_t len);

// messages
std::string	handle_redirect(std::pair<std::string, LocationStruct> location);
std::string	handle_notAllowed(std::pair<std::string, LocationStruct> location);
std::string	notFound(void);
std::string	bad_request(void);
std::string	forbidden_403(void);
std::string	internal_error(void);
std::string	created_success(void);
std::string OK_200(std::string &body);
std::string large_payload();

std::string ft_random_time();

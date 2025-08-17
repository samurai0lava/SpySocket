#pragma once

#include "webserv.hpp"

void	postMethod(int fd, string uri, ConfigStruct config, ParsingRequest &parser);

std::pair<std::string, LocationStruct> get_location(int fd,
	const std::string &requestPath, const ConfigStruct &server);

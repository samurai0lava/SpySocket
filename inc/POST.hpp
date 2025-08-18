#pragma once

#include "webserv.hpp"

string	postMethod(string uri, ConfigStruct config, ParsingRequest& parser);

std::pair<std::string, LocationStruct> get_location(const std::string& requestPath, const ConfigStruct& server);

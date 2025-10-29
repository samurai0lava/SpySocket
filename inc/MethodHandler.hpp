#pragma once
#include "webserv.hpp"
class CClient;

// void handleMethod(int client_fd, ParsingRequest* parser, const ConfigStruct& config, CClient& client_data);
void handleMethod(int client_fd, ParsingRequest* parser, std::map<std::string, ConfigStruct> config, CClient& client_data);


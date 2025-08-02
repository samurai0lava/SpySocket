#pragma once
#include "webserv.hpp"
void handleMethod(int client_fd,ParsingRequest* parser, const ConfigStruct& config,Servers &serv);
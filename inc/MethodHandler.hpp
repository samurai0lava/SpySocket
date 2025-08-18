#pragma once
#include "webserv.hpp"
string handleMethod(int client_fd,ParsingRequest* parser, const ConfigStruct& config,Servers &serv);

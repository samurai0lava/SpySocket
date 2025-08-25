#pragma once
#include "webserv.hpp"
class CClient;
string handleMethod(int client_fd,ParsingRequest* parser, const ConfigStruct& config,Servers &serv,CClient &client_data);

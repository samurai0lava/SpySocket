#pragma once
#include "webserv.hpp"
class CClient;
void handleMethod(int client_fd,ParsingRequest* parser, const ConfigStruct& config,Servers &serv,CClient &client_data);

#pragma once
#include "webserv.hpp"
// #include "ConfigStruct.hpp"

void handleMethod(int fd, ParsingRequest* parser, const ConfigStruct& config,Servers &serv);
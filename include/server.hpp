#ifndef SERVER_HPP
# define SERVER_HPP


#pragma once
#include <sys/epoll.h>
#include <iostream>
#include <vector>
#include "Config.hpp"
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <algorithm>

using namespace std;

class Servers
{
public :
    vector<int> serversFd;
    std::map<std::string, ConfigStruct> configStruct;

};

void getServersFds(Config *configFile, Servers &serv);
void epollFds(Servers& serv);

#endif
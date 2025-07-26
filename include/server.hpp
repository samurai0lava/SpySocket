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

#define READ_SIZE 1024

using namespace std;

class Servers
{
public :
    vector<int> serversFd;
    std::map<std::string, ConfigStruct> configStruct;
    char buffer[READ_SIZE];
    size_t bufferLength;

};

void getServersFds(Config *configFile, Servers &serv);
void epollFds(Servers& serv);

#endif
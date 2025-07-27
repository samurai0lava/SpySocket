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
#include <sys/stat.h>
#include <dirent.h>   // Required for DIR, opendir(), readdir(), closedir()
#include <sys/stat.h> // Required for stat()
using namespace std;

class Servers
{
    public :
        vector<int> serversFd;
        std::map<std::string, ConfigStruct> configStruct;
        void MethodGet(std::map<std::string, ConfigStruct> configStruct);
        

};
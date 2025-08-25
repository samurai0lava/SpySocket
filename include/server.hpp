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
#include "../inc/CClient.hpp"
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>   // Required for DIR, opendir(), readdir(), closedir()
#include <sys/stat.h> // Required for stat()
#include <map>
#define READ_SIZE 1024

using namespace std;

struct Client {
    int fd;
    // std::string request;
    std::string response;
    bool ready_to_respond;
};

class Servers
{
public:
    std::map<int, ClientSendState> clientSendStates;
    char buffer[READ_SIZE];
    ssize_t bufferLength;
    Servers() : bufferLength(0) {
        memset(buffer, 0, READ_SIZE);
    }
    // ~Servers() {
    //     for (vector<int>::iterator it = serversFd.begin(); it != serversFd.end(); ++it) {
    //         close(*it);
    //     }
    // }
    char* getBuffer() {
        return buffer;
    }
    size_t getBufferLength() const {
        return bufferLength;
    }
    vector<int> serversFd;
    // Add this inside your class
    std::vector<int>& getServersFds() {
        return serversFd;
    }
    std::map<std::string, ConfigStruct> configStruct;
    void getServersFds(Config* configFile, Servers& serv);
    void epollFds(Servers& serv);

};


#endif

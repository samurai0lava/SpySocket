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
#include <sys/stat.h>
#include <dirent.h>
#include <sys/stat.h>
#include <map>
#include <signal.h>
#define READ_SIZE 65536  // 64kb

void ft_memset(void* b, int c, size_t len);

struct Client {
    int fd;
    std::string response;
    bool ready_to_respond;
};

class Servers
{
private:
    bool is_running;
    static Servers* instance;
    Servers() : is_running(false), bufferLength(0) {
        ft_memset(buffer, 0, READ_SIZE);
    }
    Servers(const Servers&);
    Servers& operator=(const Servers&);

public:
    char buffer[READ_SIZE];
    size_t bufferLength;

    static Servers* getInstance() {
        if (instance == NULL) {
            instance = new Servers();
        }
        return instance;
    }

    static void destroyInstance() {
        if (instance != NULL) {
            delete instance;
            instance = NULL;
        }
    }
    ~Servers() {
        for (std::vector<int>::iterator it = serversFd.begin(); it != serversFd.end(); ++it) {
            close(*it);
        }
    }
    char* getBuffer() {
        return buffer;
    }
    size_t getBufferLength() const {
        return bufferLength;
    }
    std::vector<int> serversFd;
    std::vector<int>& getServersFds() {
        return serversFd;
    }
    std::map<std::string, ConfigStruct> configStruct;
    void getServersFds(Config* configFile, Servers& serv);
    void epollFds(Servers& serv);
    void setIsRunning(bool state) { is_running = state; }
    bool getIsRunning() const { return is_running; }
};

void handle_signal();

#endif
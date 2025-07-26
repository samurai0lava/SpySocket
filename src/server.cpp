#include "../include/server.hpp"

void getServersFds(Config *configFile, Servers &serv)
{
    // Servers serv;
    serv.configStruct = configFile->_cluster;
    // int serversCount = serv.configStruct.size();

    int serverFd;

    for (std::map<std::string, ConfigStruct>::iterator it = serv.configStruct.begin(); it != serv.configStruct.end(); it++)
    {
        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in sockStruct;
        sockStruct.sin_family = AF_INET;
        sockStruct.sin_addr.s_addr = inet_addr((*it).second.host.c_str()); // localhost e.g
        unsigned short port = *((*it).second.listen.begin());
        sockStruct.sin_port = htons(port); // check if the port is valid

        bind(serverFd, (sockaddr *)&sockStruct, sizeof(sockStruct));
        listen(serverFd, 10);
        std::cout << "Listening on " << it->second.host << ":" << port << " (fd=" << serverFd << ")\n";
        serv.serversFd.push_back(serverFd);
    }
}

int setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        perror("fcntl F_SETFL");
        return -1;
    }
    return 0;
}

const char *http_response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Hello, World!";

void epollFds(Servers &serv)
{
    int epollFd = epoll_create1(0);
    if (epollFd == -1)
        throw runtime_error("Error creating epoll!");

    struct epoll_event event;
    for (vector<int>::iterator it = serv.serversFd.begin(); it != serv.serversFd.end(); it++)
    {
        if (setNonBlocking(*it) == -1)
        {
            cerr << "Error : Cannot set non blocking mode on : " << *it << " (server's fd)!\n";
            close(*it);
            continue;
        }

        event.events = EPOLLIN;
        event.data.fd = *it;

        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, *it, &event) == -1)
        {
            cerr << "Cannot add server " << *it << " to epoll!\n";
            close(*it);
            continue;
        }
    }

    struct epoll_event events[10];

    while (true)
    {
        int nfds = epoll_wait(epollFd, events, 10, -1);
        if (nfds == -1)
        {
            cerr << "Error occured in epoll wait!\n";
            break;
        }

        for (int i = 0; i < nfds; ++i)
        {
            int fd = events[i].data.fd;

            // Check if fd is a listening socket
            std::vector<int>::iterator it = std::find(serv.serversFd.begin(), serv.serversFd.end(), fd);
            if (it != serv.serversFd.end())
            {
                int client_fd = accept(fd, NULL, NULL);
                if (client_fd == -1)
                {
                    cerr << "Error accepting client's connection!\n";
                    continue;
                }

                if (setNonBlocking(client_fd) == -1)
                {
                    cerr << "Cannot set client to non blocking!\n";
                    close(client_fd);
                    continue;
                }

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;

                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, client_fd, &event) == -1)
                {
                    cerr << "Error adding client to epoll!\n";
                    close(client_fd);
                }

                std::cout << "New client connected on FD " << client_fd << std::endl;
            }
            else
            {
                // Handle client socket
                // serv.buffer;
                memset(serv.buffer, 0, READ_SIZE) ;
                serv.bufferLength = recv(fd, serv.buffer, READ_SIZE, MSG_WAITALL);
                // char buffer[1024] = {0};
                // ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
                if (serv.bufferLength <= 0)
                {
                    if (serv.bufferLength == 0)
                        std::cout << "Client disconnected.\n";
                    else
                        cerr << "Error occured while reading sent data!\n";

                    close(fd);
                    continue;
                }
                
                //Request
                std::cout << "Received: " << serv.buffer << std::endl;
                // Response
                // write(fd, http_response, strlen(http_response));
            }
        }
    }

    close(epollFd);
}

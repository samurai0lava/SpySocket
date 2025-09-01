#include "../include/server.hpp"
#include "../inc/webserv.hpp"

void Servers::getServersFds(Config* configFile, Servers& serv)
{
    // Servers serv;
    serv.configStruct = configFile->_cluster;
    // int serversCount = serv.configStruct.size();

    int serverFd;

    for (std::map<std::string, ConfigStruct>::iterator it = serv.configStruct.begin(); it != serv.configStruct.end(); it++)
    {
        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd == -1) {
            perror("socket creation failed");
            continue;
        }

        // Set SO_REUSEADDR to avoid "Address already in use" errors
        int opt = 1;
        if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt SO_REUSEADDR failed");
            close(serverFd);
            continue;
        }

        sockaddr_in sockStruct;
        sockStruct.sin_family = AF_INET;
        sockStruct.sin_addr.s_addr = inet_addr((*it).second.host.c_str()); // localhost e.g
        unsigned short port = *((*it).second.listen.begin());
        sockStruct.sin_port = htons(port); // check if the port is valid

            
        if (bind(serverFd, (sockaddr*)&sockStruct, sizeof(sockStruct)) < 0) {
            perror("bind failed");
            close(serverFd);
            continue;
        }

        if (listen(serverFd, SOMAXCONN) < 0) {
            perror("listen failed");
            close(serverFd);
            continue;
        }

        std::cout << BLUE "Listening on " RESET << it->second.host << ":" << port << " (fd=" << serverFd << ")\n";
        serv.serversFd.push_back(serverFd);
    }
}

int set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return -1;

    return 0;
}


void Servers::epollFds(Servers& serv)
{
    int epollFd = epoll_create1(0);
    if (epollFd == -1)
        throw runtime_error("Error creating epoll!");

    struct epoll_event event;
    for (vector<int>::iterator it = serv.serversFd.begin(); it != serv.serversFd.end(); it++)
    {
        if (set_non_blocking(*it) == -1)
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

    std::map<int, Client> clients;
    struct epoll_event events[10];
    std::map<int, ParsingRequest*> clientParsers;  // Map client FD to parser instance


    while (true)
    {
        int ready_fds = epoll_wait(epollFd, events, 10, -1);
        if (ready_fds == -1)
        {
            std::cerr << "Error occured in epoll wait!" << std::endl;
            break;
        }

        for (int i = 0; i < ready_fds; ++i)
        {
            int fd = events[i].data.fd;

            // Check if fd is a listening socket (*it) is a server socket fd
            std::vector<int>::iterator it = std::find(serv.serversFd.begin(), serv.serversFd.end(), fd);
            if (it != serv.serversFd.end())
            {
                int client_fd = accept(fd, NULL, NULL);
                if (client_fd == -1)
                {
                    std::cerr << "Error accepting client's connection!" << std::endl;
                    continue;
                }

                if (set_non_blocking(client_fd) == -1)
                {
                    std::cerr << "Cannot set client to non blocking!\n";
                    close(client_fd);
                    continue;
                }

                epoll_event client_ev;
                client_ev.events = EPOLLIN;
                client_ev.data.fd = client_fd;
                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1)
                {
                    std::cerr << "Error adding client to epoll!" << std::endl;
                    close(client_fd);
                }
                else
                {
                    Client tmp;
                    tmp.fd = client_fd;
                    tmp.response = "";
                    tmp.ready_to_respond = false;
                    clients[client_fd] = tmp;
                    clientParsers[client_fd] = new ParsingRequest();
                    std::cout << "New client connected on FD " << client_fd << std::endl;
                }
                continue;
            }

            Client& c = clients[fd];
            string body_file = "";
            if (events[i].events & EPOLLIN)
            {
                memset(&serv.buffer, 0, READ_SIZE);
                
                //READ_SIZE = 8000
                serv.bufferLength = recv(fd, serv.buffer, READ_SIZE, 0);
                if (serv.bufferLength <= 0)
                {
                    if (serv.bufferLength == 0)
                    std::cout << "Client disconnected.\n";
                    else
                    cerr << "Error occured while reading sent data!\n";
                    
                    if (clientParsers.find(fd) != clientParsers.end())
                    {
                        delete clientParsers[fd];
                        clientParsers.erase(fd);
                    }
                    close(fd);
                    continue;
                }

                // Get the parser for this specific client
                ParsingRequest* parser = NULL;
                if (clientParsers.find(fd) != clientParsers.end())
                {
                    parser = clientParsers[fd];
                }
                else
                {
                    std::cerr << "No parser found for client FD " << fd << std::endl;
                    close(fd);
                    continue;
                }

                ParsingRequest::ParseResult result = parser->feed_data(serv.buffer, serv.bufferLength);

                if (result == ParsingRequest::PARSE_OK)
                {
                    printRequestInfo(*parser, fd);
                    ConfigStruct& config = serv.configStruct.begin()->second;
                    c.response = handleMethod(fd, parser, config, serv); 
                    c.ready_to_respond = true;
                    epoll_event ev;
                    ev.events = EPOLLOUT;
                    ev.data.fd = fd;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);

                    parser->reset();
                }
                else if (result == ParsingRequest::PARSE_AGAIN)
                {
                    std::cout << YELLOW "Waiting for more data on fd ... " RESET << fd << std::endl;
                }
                else if (result == ParsingRequest::PARSE_ERROR_RESULT)
                {
                    // Handle any error result - send the error response
                    std::cout << "Error Code: " << parser->getErrorCode() << " - " << parser->getErrorMessage() << std::endl;
                    std::string errorResponse = GenerateResErr(parser->getErrorCode());
                    c.response = errorResponse;
                    c.ready_to_respond = true;
                    epoll_event ev;
                    ev.events = EPOLLOUT;
                    ev.data.fd = fd;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
                    // send(fd, errorResponse.c_str(), errorResponse.length(), 0);
                    delete clientParsers[fd];
                    clientParsers.erase(fd);
                    close(fd);
                }

            }
            else if (events[i].events & EPOLLOUT)
            {
                size_t bytes_sent = send(fd, c.response.c_str(), c.response.size(), 0);
                if (bytes_sent > 0)
                    c.response.erase(0, bytes_sent);
                if (c.response.empty())
                {
                    c.ready_to_respond = false;
                    epoll_event ev;
                    ev.events = EPOLLIN;
                    ev.data.fd = fd;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
                }
            }


        }
    }

    // Clean up all parsers
    for (std::map<int, ParsingRequest*>::iterator it = clientParsers.begin();
        it != clientParsers.end(); ++it) {
        delete it->second;
    }
    clientParsers.clear();

    close(epollFd);
}
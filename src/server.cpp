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
    memset(&event, 0, sizeof(event));
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
    // CClient client_data;
    std::map<int, CClient> client_data_map;
    struct epoll_event events[10];
    std::map<int, ParsingRequest*> clientParsers;

    int i = -1;

    while (true)
    {
        i++;
        //epoll wait returns 2 fds that are ready when only one client is connected ???
        int ready_fds = epoll_wait(epollFd, events, 10, -1);

        // std::cout << "READY FDS : " << ready_fds << std::endl;

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
                memset(&client_ev, 0, sizeof(client_ev));
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
                    client_data_map[client_fd] = CClient();
                    client_data_map[client_fd].FdClient = client_fd;
                    std::cout << "New client connected on FD " << client_fd << std::endl;
                }
                continue;
            }

            // Check if client still exists in our maps
            if (clients.find(fd) == clients.end()) {
                std::cout << "Client fd " << fd << " no longer exists, skipping event" << std::endl;
                continue;
            }

            Client& c = clients[fd];
            if (events[i].events & EPOLLIN)
            {
                serv.bufferLength = recv(fd, serv.buffer, READ_SIZE, 0);
                // cout << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";
                // cout << serv.buffer;
                // cout << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";
                if (serv.bufferLength <= 0)
                {
                    if (serv.bufferLength == 0)
                        std::cout << "Client disconnected.";
                    else
                        cerr << "Error occured while reading sent data!";
                    
                    // Proper cleanup of all client data structures
                    if (clientParsers.find(fd) != clientParsers.end()) {
                        delete clientParsers[fd];
                        clientParsers.erase(fd);
                    }
                    if (clients.find(fd) != clients.end()) {
                        clients.erase(fd);
                    }
                    if (client_data_map.find(fd) != client_data_map.end()) {
                        client_data_map.erase(fd);
                    }
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
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
                    access_error(500, "Internal Server Error: No parser found for client.");
                    close(fd);
                    continue;
                }

                ParsingRequest::ParseResult result = parser->feed_data(serv.buffer, serv.bufferLength);

                if (result == ParsingRequest::PARSE_OK)
                {
                    // printRequestInfo(*parser, fd);
                    ConfigStruct& config = serv.configStruct.begin()->second;
                    // access_log(*parser);
                    handleMethod(fd, parser, config, serv, client_data_map[fd]);
                    c.ready_to_respond = true;
                    epoll_event ev;
                    memset(&ev, 0, sizeof(ev));
                    ev.events = EPOLLOUT;
                    ev.data.fd = fd;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);

                    // Don't reset parser here - reset it after response is sent
                    // parser->reset();
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
                    memset(&ev, 0, sizeof(ev));
                    ev.events = EPOLLOUT;
                    ev.data.fd = fd;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
                    // Proper cleanup
                    if (clientParsers.find(fd) != clientParsers.end()) {
                        delete clientParsers[fd];
                        clientParsers.erase(fd);
                    }
                    // Note: Don't erase from other maps here as we still need to send the response
                }

            }
            else if (events[i].events & EPOLLOUT)
            {
                // Check if client still exists in our maps
                if (client_data_map.find(fd) == client_data_map.end() || 
                    clients.find(fd) == clients.end()) {
                    std::cout << "Client fd " << fd << " no longer exists, skipping EPOLLOUT" << std::endl;
                    continue;
                }
                
                if (client_data_map[fd].NameMethod == "GET")
                {
                    if (c.response.empty() && client_data_map[fd].chunkedSending == false) {
                        // std::cout<<"****Building response for fd : "<< fd << std::endl;
                            // Build response only once (headers + first chunk)
                        c.response = client_data_map[fd].HandleAllMethod();
                    }
                    ssize_t bytes_sent = send(fd, c.response.c_str(), c.response.size(), MSG_NOSIGNAL);
                    if (bytes_sent == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Socket not ready yet, try again later
                            continue;
                        } else {
                            // Connection error, clean up this client
                            std::cout << "Send failed for fd " << fd << ", cleaning up" << std::endl;
                            if (clientParsers.find(fd) != clientParsers.end()) {
                                delete clientParsers[fd];
                                clientParsers.erase(fd);
                            }
                            clients.erase(fd);
                            client_data_map.erase(fd);
                            epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                            continue;
                        }
                    }
                    if (bytes_sent > 0)
                    {
                        client_data_map[fd].bytesSent += bytes_sent;
                        c.response.erase(0, bytes_sent);
                    }
                    if (c.response.empty())
                    {
                        c.ready_to_respond = false;
                        if (client_data_map[fd].chunkedSending == true)
                        {
                            std::cout << "Finished sending response to fd : " << fd << std::endl;
                            epoll_event ev;
                            memset(&ev, 0, sizeof(ev));
                            ev.events = EPOLLIN; // Reset to listen for new requests
                            ev.data.fd = fd;
                            client_data_map[fd] = CClient();
                            client_data_map[fd].FdClient = fd;
                            if (clientParsers.find(fd) != clientParsers.end()) {
                                clientParsers[fd]->reset();
                            }
                            epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
                        }
                        // else
                        // {
                        //     // For non-chunked responses, also reset for keep-alive
                        //     epoll_event ev;
                        //     memset(&ev, 0, sizeof(ev));
                        //     ev.events = EPOLLIN;
                        //     ev.data.fd = fd;
                        //     client_data_map[fd] = CClient();
                        //     client_data_map[fd].FdClient = fd;
                        //     // Reset parser for next request on keep-alive connection
                        //     if (clientParsers.find(fd) != clientParsers.end()) {
                        //         clientParsers[fd]->reset();
                        //     }
                        //     epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
                        // }

                    }
                }
                else
                {
                    // std::cout << "Ready to send response to fd : " << fd << std::endl;
                    if (c.response.empty())
                        c.response = client_data_map[fd].HandleAllMethod();
                    ssize_t bytes_sent = send(fd, c.response.c_str(), c.response.size(), MSG_NOSIGNAL);
                    if (bytes_sent == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Socket not ready yet, try again later
                            continue;
                        } else {
                            // Connection error, clean up this client
                            std::cout << "Send failed for fd " << fd << ", cleaning up" << std::endl;
                            if (clientParsers.find(fd) != clientParsers.end()) {
                                delete clientParsers[fd];
                                clientParsers.erase(fd);
                            }
                            clients.erase(fd);
                            client_data_map.erase(fd);
                            epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                            continue;
                        }
                    }
                    if (bytes_sent > 0)
                    {
                        client_data_map[fd].bytesSent += bytes_sent;
                        c.response.erase(0, bytes_sent);
                    }
                    if (c.response.empty())
                    {
                        c.ready_to_respond = false;
                        std::cout << "Finished sending response to fd : " << fd << std::endl;
                        epoll_event ev;
                        memset(&ev, 0, sizeof(ev));
                        ev.events = EPOLLIN; // Reset to listen for new requests
                        ev.data.fd = fd;
                        // Reset client data for keep-alive instead of erasing
                        client_data_map[fd] = CClient();
                        client_data_map[fd].FdClient = fd;
                        // Reset parser for next request on keep-alive connection
                        if (clientParsers.find(fd) != clientParsers.end()) {
                            clientParsers[fd]->reset();
                        }
                        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
                    }
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
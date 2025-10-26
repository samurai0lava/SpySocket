#include "../include/server.hpp"
#include "../inc/webserv.hpp"
#include <sys/wait.h>

Servers* Servers::instance = NULL;

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
        if(DEBUG_MODE == 1)
            access_start_server(port);
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
        throw std::runtime_error("Error creating epoll!");
    struct epoll_event event;
    ft_memset(&event, 0, sizeof(event));
    for (std::vector<int>::iterator it = serv.serversFd.begin(); it != serv.serversFd.end(); it++)
    {
        if (set_non_blocking(*it) == -1)
        {
            std::cerr << "Error : Cannot set non blocking mode on : " << *it << " (server's fd)!\n";
            close(*it);
            continue;
        }

        event.events = EPOLLIN;
        event.data.fd = *it;

        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, *it, &event) == -1)
        {
            std::cerr << "Cannot add server " << *it << " to epoll!\n";
            close(*it);
            continue;
        }
    }

    std::map<int, Client> clients;
    // CClient client_data;
    std::map<int, CClient> client_data_map;
    std::map<int, int> cgi_fd_to_client_fd; // Map CGI fd to client fd
    struct epoll_event events[10];
    std::map<int, ParsingRequest*> clientParsers;

    int i = -1;
    is_running = true;
    while (is_running)
    {
        i++;
        int ready_fds = epoll_wait(epollFd, events, 10, EPOLL_TIMEOUT);
        if (ready_fds == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                access_error(500, "Internal Server Error: epoll_wait failed.");
                perror("epoll_wait");
                break;
            }
        }
        std::vector<int> timed_out_clients;
        std::vector<int> failed_cgi_clients;

        for (std::map<int, CClient>::iterator it = client_data_map.begin(); it != client_data_map.end(); ++it) {
            int client_fd = it->first;
            CClient& client_data = it->second;

            if (client_data.cgi_handler) {
                pid_t cgi_pid = client_data.cgi_handler->get_cgi_pid();
                if (cgi_pid > 0) {
                    int status;
                    pid_t result = waitpid(cgi_pid, &status, WNOHANG);

                    if (result == cgi_pid && (!WIFEXITED(status) || WEXITSTATUS(status) != 0)) {
                        failed_cgi_clients.push_back(client_fd);
                        continue;
                    }
                }
                if (client_data.cgi_handler->is_cgi_timeout(CGI_TIMEOUT)) {
                    timed_out_clients.push_back(client_fd);
                }
            }
        }

        // Handle CGI scripts that failed (syntax errors, etc.)
        for (size_t i = 0; i < failed_cgi_clients.size(); i++) {
            int client_fd = failed_cgi_clients[i];
            if (clients.find(client_fd) == clients.end()) {
                continue;
            }
            CClient& client_data = client_data_map[client_fd];

            int cgi_fd = -1;
            if (client_data.cgi_handler) {
                cgi_fd = client_data.cgi_handler->get_cgi_fd();
                client_data.cgi_handler->close_cgi();
                delete client_data.cgi_handler;
                client_data.cgi_handler = NULL;
            }

            if (cgi_fd >= 0) {
                epoll_ctl(epollFd, EPOLL_CTL_DEL, cgi_fd, NULL);
                cgi_fd_to_client_fd.erase(cgi_fd);
            }

            client_data.is_cgi_request = false;
            clients[client_fd].response = GenerateResErr(502); // Bad Gateway
            clients[client_fd].ready_to_respond = true;

            epoll_event ev;
            ft_memset(&ev, 0, sizeof(ev));
            ev.events = EPOLLOUT;
            ev.data.fd = client_fd;
            epoll_ctl(epollFd, EPOLL_CTL_MOD, client_fd, &ev);
        }

        for (size_t i = 0; i < timed_out_clients.size(); i++) {
            int client_fd = timed_out_clients[i];
            if (clients.find(client_fd) == clients.end()) {
                continue;
            }
            CClient& client_data = client_data_map[client_fd];
            std::cerr << "CGI timeout for client fd " << client_fd << std::endl;
            int cgi_fd = -1;
            if (client_data.cgi_handler) {
                cgi_fd = client_data.cgi_handler->get_cgi_fd();
                client_data.cgi_handler->close_cgi();
                delete client_data.cgi_handler;
                client_data.cgi_handler = NULL;
            }
            if (cgi_fd >= 0) {
                epoll_ctl(epollFd, EPOLL_CTL_DEL, cgi_fd, NULL);
                cgi_fd_to_client_fd.erase(cgi_fd);
            }

            client_data.is_cgi_request = false;
            clients[client_fd].response = GenerateResErr(504);
            clients[client_fd].ready_to_respond = true;

            epoll_event ev;
            ft_memset(&ev, 0, sizeof(ev));
            ev.events = EPOLLOUT;
            ev.data.fd = client_fd;
            epoll_ctl(epollFd, EPOLL_CTL_MOD, client_fd, &ev);
        }
        for (int i = 0; i < ready_fds; ++i)
        {
            int fd = events[i].data.fd;

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
                ft_memset(&client_ev, 0, sizeof(client_ev));
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
                    // std::cout << "New client connected on FD " << client_fd << std::endl;
                }
                continue;
            }
            if (clients.find(fd) == clients.end()) {
                if (cgi_fd_to_client_fd.find(fd) != cgi_fd_to_client_fd.end()) {
                    int client_fd = cgi_fd_to_client_fd[fd];
                    if (clients.find(client_fd) != clients.end() &&
                        client_data_map.find(client_fd) != client_data_map.end()) {
                        CClient& client_data = client_data_map[client_fd];
                        if (client_data.cgi_handler && events[i].events & EPOLLIN) {
                            client_data.cgi_handler->read_output();
                            std::string cgi_response = client_data.HandleCGIMethod();
                            if (!cgi_response.empty()) {
                                clients[client_fd].response = cgi_response;
                                clients[client_fd].ready_to_respond = true;
                                epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
                                cgi_fd_to_client_fd.erase(fd);
                                epoll_event ev;
                                ft_memset(&ev, 0, sizeof(ev));
                                ev.events = EPOLLOUT;
                                ev.data.fd = client_fd;
                                epoll_ctl(epollFd, EPOLL_CTL_MOD, client_fd, &ev);
                            }
                        }
                    }
                    continue;
                }
                continue;
            }

            Client& c = clients[fd];
            if (events[i].events & EPOLLIN)
            {
                //Timeout for the request
                // ft_memset(serv.buffer, 0, READ_SIZE);
                serv.bufferLength = recv(fd, serv.buffer, READ_SIZE, 0);
                // cout << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";
                // cout << serv.buffer;
                // cout << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";
                if (serv.bufferLength <= 0)
                {
                    if (serv.bufferLength == 0) {
                        std::cout << "Client disconnected on fd " << fd << std::endl;
                    } else {
                        std::cerr << "Error occurred while reading from fd " << fd << ": " << strerror(errno) << std::endl;
                    }

                    // Clean up CGI resources if any
                    if (client_data_map.find(fd) != client_data_map.end()) {
                        CClient& client_data = client_data_map[fd];
                        if (client_data.cgi_handler) {
                            int cgi_fd = client_data.cgi_handler->get_cgi_fd();
                            if (cgi_fd >= 0) {
                                epoll_ctl(epollFd, EPOLL_CTL_DEL, cgi_fd, NULL);
                                cgi_fd_to_client_fd.erase(cgi_fd);
                            }
                        }
                    }

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
                    // access_error(500, "Internal Server Error: No parser found for client.");
                    close(fd);
                    continue;
                }

                ParsingRequest::ParseResult result = parser->feed_data(serv.buffer, serv.bufferLength);

                if (result == ParsingRequest::PARSE_OK)
                {
                    printRequestInfo(*parser, fd);
                    ConfigStruct& config = serv.configStruct.begin()->second;
                    if(DEBUG_MODE == 1)
                        access_log(*parser);
                    handleMethod(fd, parser, config, client_data_map[fd]);
                    // If it's a CGI request, set up the CGI fd in epoll
                    if (client_data_map[fd].is_cgi_request && client_data_map[fd].cgi_handler) {
                        int cgi_fd = client_data_map[fd].cgi_handler->get_cgi_fd();
                        if (cgi_fd >= 0) {
                            epoll_event cgi_ev;
                            ft_memset(&cgi_ev, 0, sizeof(cgi_ev));
                            cgi_ev.events = EPOLLIN;
                            cgi_ev.data.fd = cgi_fd;
                            if (epoll_ctl(epollFd, EPOLL_CTL_ADD, cgi_fd, &cgi_ev) != -1) {
                                cgi_fd_to_client_fd[cgi_fd] = fd;
                            }
                            else {
                                std::cerr << "Failed to add CGI fd to epoll" << std::endl;
                            }
                        }
                        continue;
                    }
                    else {
                        c.ready_to_respond = true;
                        epoll_event ev;
                        ft_memset(&ev, 0, sizeof(ev));
                        ev.events = EPOLLOUT;
                        ev.data.fd = fd;
                        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
                    }
                }
                else if (result == ParsingRequest::PARSE_AGAIN)
                {
                    std::cout << YELLOW "Waiting for more data on fd ... " RESET << fd << std::endl;
                }
                else if (result == ParsingRequest::PARSE_ERROR_RESULT)
                {
                    std::cout << "Error Code: " << parser->getErrorCode() << " - " << parser->getErrorMessage() << std::endl;
                    std::string errorResponse = GenerateResErr(parser->getErrorCode());
                    c.response = errorResponse;
                    c.ready_to_respond = true;
                    // Mark that connection should be closed after sending error response
                    client_data_map[fd].should_close_connection = true;
                    epoll_event ev;
                    ft_memset(&ev, 0, sizeof(ev));
                    ev.events = EPOLLOUT;
                    ev.data.fd = fd;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
                    if (clientParsers.find(fd) != clientParsers.end()) {
                        delete clientParsers[fd];
                        clientParsers.erase(fd);
                    }
                }

            }
            else if (events[i].events & EPOLLOUT)
            {
                //timeout for the response?
                if (client_data_map.find(fd) == client_data_map.end() ||
                    clients.find(fd) == clients.end()) {
                    continue;
                }

                if (client_data_map[fd].NameMethod == "GET")
                {
                    if (c.response.empty() && client_data_map[fd].chunkedSending == false) {
                        c.response.clear();
                        c.response = client_data_map[fd].HandleAllMethod();
                    }
                    ssize_t bytes_sent = send(fd, c.response.c_str(), c.response.size(), MSG_NOSIGNAL);
                    if (bytes_sent == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            continue;
                        }
                        else if (bytes_sent == 0) {
                            std::cout << "Send failed for fd " << fd << ", cleaning up1" << std::endl;
                            // Clean up CGI resources if any
                            if (client_data_map.find(fd) != client_data_map.end()) {
                                CClient& client_data = client_data_map[fd];
                                if (client_data.cgi_handler) {
                                    int cgi_fd = client_data.cgi_handler->get_cgi_fd();
                                    if (cgi_fd >= 0) {
                                        epoll_ctl(epollFd, EPOLL_CTL_DEL, cgi_fd, NULL);
                                        cgi_fd_to_client_fd.erase(cgi_fd);
                                    }
                                }
                            }
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
                        if (static_cast<size_t>(bytes_sent) <= c.response.size()) {
                            c.response.erase(0, bytes_sent);
                        }
                        else {
                            std::cerr << "WARNING: bytes_sent (" << bytes_sent << ") > response.size() ("
                                << c.response.size() << "), clearing response" << std::endl;
                            c.response.clear();
                        }
                    }
                    if (c.response.empty())
                    {
                        c.ready_to_respond = false;

                        // Check if connection should be closed
                        bool should_close = false;
                        if (client_data_map[fd].should_close_connection) {
                            should_close = true;
                        } else if (clientParsers.find(fd) != clientParsers.end() &&
                                   clientParsers[fd]->getConnectionStatus() == 0) {
                            should_close = true;
                        }

                        if (should_close) {
                            // Close the connection
                            std::cout << "Closing connection for fd " << fd << " (connection: close or error)" << std::endl;
                            if (clientParsers.find(fd) != clientParsers.end()) {
                                delete clientParsers[fd];
                                clientParsers.erase(fd);
                            }
                            clients.erase(fd);
                            client_data_map.erase(fd);
                            epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                        }
                        else if (client_data_map[fd].chunkedSending == true)
                        {
                            // std::cout << GREEN "Finished sending response to fd : " << fd << RESET << std::endl;
                            epoll_event ev;
                            ft_memset(&ev, 0, sizeof(ev));
                            ev.events = EPOLLIN; // Reset to listen for new requests
                            ev.data.fd = fd;
                            client_data_map[fd] = CClient();
                            client_data_map[fd].FdClient = fd;
                            if (clientParsers.find(fd) != clientParsers.end()) {
                                clientParsers[fd]->reset();
                            }
                            epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
                        }
                        else
                        {
                            // For non-chunked responses, also reset for keep-alive
                            epoll_event ev;
                            ft_memset(&ev, 0, sizeof(ev));
                            ev.events = EPOLLIN;
                            ev.data.fd = fd;
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
                else
                {
                    // std::cout << "Ready to send response to fd : " << fd << std::endl;
                    if (c.response.empty())
                        c.response = client_data_map[fd].HandleAllMethod();
                    // cout << c.response() << endl;
                    ssize_t bytes_sent = send(fd, c.response.c_str(), c.response.size(), MSG_NOSIGNAL);
                    if (bytes_sent == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Socket not ready yet, try again later
                            continue;
                        }
                        else {
                            // Connection error, clean up this client
                            std::cout << GREEN "Send failed for fd " << fd << ", cleaning up" RESET << std::endl;
                            // Clean up CGI resources if any
                            if (client_data_map.find(fd) != client_data_map.end()) {
                                CClient& client_data = client_data_map[fd];
                                if (client_data.cgi_handler) {
                                    int cgi_fd = client_data.cgi_handler->get_cgi_fd();
                                    if (cgi_fd >= 0) {
                                        epoll_ctl(epollFd, EPOLL_CTL_DEL, cgi_fd, NULL);
                                        cgi_fd_to_client_fd.erase(cgi_fd);
                                    }
                                }
                            }
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
                        if (static_cast<size_t>(bytes_sent) <= c.response.size()) {
                            c.response.erase(0, bytes_sent);
                        }
                        else {
                            std::cerr << "WARNING: bytes_sent (" << bytes_sent << ") > response.size() ("
                                << c.response.size() << "), clearing response" << std::endl;
                            c.response.clear();
                        }
                    }
                    if (c.response.empty())
                    {
                        c.ready_to_respond = false;
                        // std::cout << "Finished sending response to fd : " << fd << std::endl;

                        // Check if connection should be closed
                        bool should_close = false;
                        if (client_data_map[fd].should_close_connection) {
                            should_close = true;
                        } else if (clientParsers.find(fd) != clientParsers.end() &&
                                   clientParsers[fd]->getConnectionStatus() == 0) {
                            should_close = true;
                        }

                        if (should_close) {
                            // Close the connection
                            std::cout << "Closing connection for fd " << fd << " (connection: close or error)" << std::endl;
                            if (clientParsers.find(fd) != clientParsers.end()) {
                                delete clientParsers[fd];
                                clientParsers.erase(fd);
                            }
                            clients.erase(fd);
                            client_data_map.erase(fd);
                            epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                        }
                        else {
                            epoll_event ev;
                            ft_memset(&ev, 0, sizeof(ev));
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
    }

    // Clean up all parsers
    for (std::map<int, ParsingRequest*>::iterator it = clientParsers.begin();
        it != clientParsers.end(); ++it) {
        delete it->second;
    }
    clientParsers.clear();

    close(epollFd);
}

void epollFds(Servers &serv, Config *conf)
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

        event.events = EPOLLIN; // level-triggered for listening sockets
        event.data.fd = *it;

        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, *it, &event) == -1)
        {
            cerr << "Cannot add server " << *it << " to epoll!\n";
            close(*it);
            continue;
        }
    }

    std::vector<epoll_event> events(64); // dynamic array, can handle more events at once
    std::map<int, ParsingRequest*> clientParsers;  // Map client FD to parser instance

    while (true)
    {
        int nfds = epoll_wait(epollFd, events.data(), events.size(), -1);
        if (nfds == -1)
        {
            std::cerr << "Error occurred in epoll wait!" << std::endl;
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
                    std::cerr << "Error accepting client's connection!" << std::endl;
                    continue;
                }

                if (setNonBlocking(client_fd) == -1)
                {
                    std::cerr << "Cannot set client to non blocking!\n";
                    close(client_fd);
                    continue;
                }

                // Edge-triggered so we must read until EAGAIN each time
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;

                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, client_fd, &event) == -1)
                {
                    std::cerr << "Error adding client to epoll!" << std::endl;
                    close(client_fd);
                }
                else
                {
                    clientParsers[client_fd] = new ParsingRequest();
                    std::cout << "New client connected on FD " << client_fd << std::endl;
                }
            }
            else
            {
                bool client_closed = false;
                

                while (true) {
                    serv.bufferLength = recv(fd, serv.buffer, sizeof(serv.buffer), 0);

                    if (serv.bufferLength > 0) {
                        // Get parser for this client
                        ParsingRequest* parser = NULL;
                        if (clientParsers.find(fd) != clientParsers.end()) {
                            parser = clientParsers[fd];
                        } else {
                            cerr << "No parser found for client FD " << fd << std::endl;
                            client_closed = true;
                            break;
                        }

                        // Feed into parser
                        ParsingRequest::ParseResult result = parser->feed_data(buf, bytes_read);

                        // If request complete, handle and reset parser
                        if (result == ParsingRequest::PARSE_OK) {
                            std::string response = handleMethod(fd, parser, conf);
                            write(fd, response.c_str(), response.length());
                            parser->reset();
                        }

                    } else if (bytes_read == 0) {
                        // Client disconnected
                        std::cout << "Client disconnected.\n";
                        client_closed = true;
                        break;

                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // No more data to read now
                            break;
                        } else {
                            std::cerr << "Error occurred while reading data!\n";
                            client_closed = true;
                            break;
                        }
                    }
                }

                // Clean up closed connection
                if (client_closed) {
                    if (clientParsers.find(fd) != clientParsers.end()) {
                        delete clientParsers[fd];
                        clientParsers.erase(fd);
                    }
                    close(fd);
                }
            }
        }
    }

    close(epollFd);
}

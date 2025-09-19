#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <unordered_map>



void set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1) return;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = INADDR_ANY;

    set_non_blocking(server_fd);
    if(bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == -1) return;
    if(listen(server_fd, SOMAXCONN) == -1) return;

    int epoll_fd = epoll_create1(0);
    if(epoll_fd == -1) return;

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    std::unordered_map<int, Client> clients;
    epoll_event events[10];

    while(true) {
        int ready_fds = epoll_wait(epoll_fd, events, 10, -1);
        for(int i = 0; i < ready_fds; i++) {
            int event_fd = events[i].data.fd;

            // New client
            if(event_fd == server_fd) {
                int client_fd = accept(server_fd, NULL, NULL);
                if(client_fd != -1) {
                    set_non_blocking(client_fd);
                    epoll_event client_ev;
                    client_ev.events = EPOLLIN;
                    client_ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
                    clients[client_fd] = {client_fd, "", "", false};
                    std::cout << "New client: " << client_fd << std::endl;
                }
                continue;
            }

            Client &c = clients[event_fd];

            // Readable
            if(events[i].events & EPOLLIN) {
                char buffer[1024];
                ssize_t bytes = recv(event_fd, buffer, sizeof(buffer), 0);

                if(bytes <= 0) {
                    std::cout << "Client disconnected: " << event_fd << std::endl;
                    close(event_fd);
                    clients.erase(event_fd);
                    continue;
                }

                c.request.append(buffer, 0, bytes);
                // Generate response, but DO NOT send now
                c.response = handle_request(c.request);
                c.ready_to_respond = true;

                // Modify epoll to watch for EPOLLOUT
                epoll_event ev{};
                ev.events = EPOLLOUT;
                ev.data.fd = event_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event_fd, &ev);
            }
            // Writable
            else if(events[i].events & EPOLLOUT) {
                ssize_t sent = send(event_fd, c.response.c_str(), c.response.size(), 0);
                if(sent > 0) {
                    c.response.erase(0, sent); // Remove sent part
                }
                // If fully sent, go back to EPOLLIN
                if(c.response.empty()) {
                    c.request.clear();
                    c.ready_to_respond = false;
                    epoll_event ev{};
                    ev.events = EPOLLIN;
                    ev.data.fd = event_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event_fd, &ev);
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
}

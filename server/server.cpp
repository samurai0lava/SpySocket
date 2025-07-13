// simple_server.cpp
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // if (server_fd == -1) {
    //     std::cerr << "Socket creation failed.\n";
    //     return 1;
    // }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); // Port 8080
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed.\n";
        return 1;
    }

    listen(server_fd, 5);
    std::cout << "Server listening on port 8080...\n";

    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        std::cerr << "Accept failed.\n";
        return 1;
    }

    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));
    read(client_fd, buffer, sizeof(buffer) - 1);
    std::cout << "Received request:\n" << buffer << "\n";

    std::string response =
        "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";

    write(client_fd, response.c_str(), response.length());

    close(client_fd);
    close(server_fd);
    return 0;
}

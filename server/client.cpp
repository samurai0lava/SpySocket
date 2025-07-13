// simple_client.cpp
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed.\n";
        return 1;
    }

    std::string request = "GET / HTTP/1.1\nHost: localhost\r\n\r\n";
    send(sock, request.c_str(), request.length(), 0);

    char buffer[4096];
    std::memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, sizeof(buffer) - 1);
    std::cout << "Received response:\n" << buffer << "\n";

    close(sock);
    return 0;
}
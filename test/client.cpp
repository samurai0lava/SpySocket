#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>


int main(int ac , char **av)
{
    if(ac != 2)
        return(-2);

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket == -1)
        return (-1);
    sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    send(clientSocket, av[1], strlen(av[1]), 0);

    close(clientSocket);

    return (0);
}
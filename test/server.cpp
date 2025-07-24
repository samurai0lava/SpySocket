#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


// int main()
// {
//     int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if(serverSocket == -1)
//         return (-1);
    
//     sockaddr_in serverAddress;
//     serverAddress.sin_family = AF_INET;
//     serverAddress.sin_port = htons(8080);
//     serverAddress.sin_addr.s_addr = INADDR_ANY;

//     bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
//     listen(serverSocket, 5);
    
    
//     //accepting the client messages here
//     int clientSocket = accept(serverSocket, nullptr, nullptr);
//     char buffer[1024] = { 0 };
//     recv(clientSocket, buffer, sizeof(buffer), 0);
//     std::cout << "Message from client: " << buffer << std::endl;
//     close(serverSocket);
//     return 0;
// }
//allo

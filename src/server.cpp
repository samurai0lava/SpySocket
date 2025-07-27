#include "../include/server.hpp"
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
                char buffer[1024] = {0};
                ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
                if (bytes <= 0)
                {
                    if (bytes == 0)
                        std::cout << "Client disconnected.\n";
                    else
                        cerr << "Error occured while reading sent data!\n";

                    close(fd);
                    continue;
                }
                
                //Request
                std::cout << "Received: " << buffer << std::endl;
                // Response
                write(fd, http_response, strlen(http_response));
            }
        }
    }

    close(epollFd);
}
// void check_is 
int main(int argc, char **argv)
{

    Config *config = new Config();
    std::string request = "GET /test/index.html HTTP/1.1\r\n"
                      "Host: localhost\r\n"
                      "Connection: close\r\n\r\n";
    try
    {
        config->StartToSet(parseArgv(argc, argv));
        // config->printCluster();
        
        // send(serv.serversFd.begin(), request.c_str(), request.length(), 0);
        // std::cout<<"0000000000000000000000000000000000000000"<<std::endl;
        // for (vector<int>::iterator it = serv.serversFd.begin(); it != serv.serversFd.end(); it++)
        //     cout << *it << endl;
        Servers serv;
        // getServersFds(config, serv);
        // epollFds(serv);
        serv.MethodGet(config->_cluster);
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        delete config;
        return (EXIT_FAILURE);
    }
    
    // send(*serv.serversFd.begin(), request.c_str(), request.length(), 0);
    delete config;
}
std::string matchLocation(const std::string &requestPath, const ConfigStruct &server) {
    std::string path = requestPath;
    std::string removedSegment;
    std::string removedPath; // Accumulates all removed segments


    while (true) {
        std::cout << "Path : " << path << " removed : " << removedPath << std::endl;

        for (size_t i = 0; i < server.location.size(); ++i) {
            if (path == server.location[i].first) {
                
                if(server.location[i].second.allowedMethods.find("GET") == server.location[i].second.allowedMethods.end())
                {
                    throw runtime_error("Error 405 Method Not Allowed");
                }
                else 
                    cout<<"found "<<endl;
                return server.location[i].second.root +removedPath;

            }
        }

        if (path == "/")
            break;

        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash == 0) {
            removedSegment = path.substr(lastSlash); // may be "/something"
            removedPath = removedSegment + removedPath;
            path = "/";
        } else {
            removedSegment = path.substr(lastSlash);  // includes "/"
            removedPath = removedSegment + removedPath; // accumulate
            path = path.substr(0, lastSlash);
        }
    }

    return ""; //no matching location
}

bool pathExists(const std::string& path) {
    struct stat s;
    return stat(path.c_str(), &s) == 0;
}
bool isFile(const std::string& path) {
    struct stat s;
    return stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode);
}

bool isDirectory(const std::string& path) {
    struct stat s;
    return stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
}

void Servers::MethodGet(std::map<std::string, ConfigStruct> configStruct)
{
    // std::string request = "GET /tours1/index.html HTTP/1.1\r\n"
    //                   "Host: localhost\r\n"
    //                   "Connection: close\r\n\r\n";
    std::string Path = "/zahira/index.html";
    std::map<std::string, ConfigStruct>::iterator it = configStruct.begin();
    ConfigStruct &server = it->second;
    std::string matchedLocation = matchLocation(Path, server);
     if (matchedLocation.empty()) {
       std::cout << "No matching location found." << std::endl;
       return;
    }
    std::cout<<"lol matche location : "<< matchedLocation<<std::endl;
    
    if (!pathExists(matchedLocation)) {
        throw std::runtime_error("404 Not Found: Path does not exist");
    }
    if (isFile(matchedLocation)) 
    {

        std::cout << "Serve file: " << matchedLocation << std::endl;
        // 🚀 NEXT: open file and read content
        std::ifstream file(matchedLocation.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("500 Internal Server Error: Cannot open file");
          std::stringstream buffer;
        buffer << file.rdbuf();
        std::string fileContent = buffer.str();
        file.close();

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << fileContent.size() << "\r\n";
        response << "\r\n";
        response << fileContent;

        std::string finalResponse = response.str();
        std::cout<<"final Response: \n"<<finalResponse<<endl;
        // send(this->serversFd[0], finalResponse.c_str(), finalResponse.length(), 0);
        return;
            // return;
    }
    if (isDirectory(matchedLocation)) {
        std::string indexPath = matchedLocation + "/" + server.location[0].second.indexPage;

        if (!pathExists(indexPath)) {
            throw std::runtime_error("403 Forbidden: Index not found");
        }

        std::cout << "Serve index file: " << indexPath << std::endl;
        // 🚀 NEXT: open indexPath and read content
        return;
    }
    throw std::runtime_error("403 Forbidden: Unknown path type");
    // const LocationStruct& loc = 
    // if (loc.allowedMethods.find("GET") == loc.allowedMethods.end()) {
    //     sendError(405, "Method Not Allowed"); // Custom function to build error response
    // return;
    // }
    //  for (size_t i = 0; i < server.location.size(); ++i)
    // {
    //     const std::string &loc_path = server.location[i].first;
    //     const LocationStruct &loc_struct = server.location[i].second;

    //     std::cout << "Location Path: " << loc_path << std::endl;
    //     std::cout << "  → Root: " << loc_struct.root << std::endl;
    //     std::cout << "  → Index Page: " << loc_struct.indexPage << std::endl;
    //     std::cout << "  → AutoIndex: " << (loc_struct.autoIndex ? "on" : "off") << std::endl;
    //     std::cout << "  → Return: " << loc_struct._return << std::endl;

    //     // Print allowed methods
    //     std::cout << "  → Methods: ";
    //     for (std::set<std::string>::iterator m = loc_struct.allowedMethods.begin(); m != loc_struct.allowedMethods.end(); ++m)
    //         std::cout << *m << " ";
    //     std::cout << std::endl;
    // }
}
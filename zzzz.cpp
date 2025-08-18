struct stat st;
if (stat(rootPath.c_str(), &st) == 0) {
    if (S_ISDIR(st.st_mode)) {
        // it's a directory
    } else {
        // it's a file
    }
}


swirver 

#include "post.hpp"

std::string generate_filename()
{
    std::string result;
    static int counter = 0;
    result = "upload_";
    result += (char)(counter + '0');
    result += ".txt";
    counter++;
    return result;
}

void handle_request(const std::string &request)
{
    std::istringstream request_stream(request);
    std::string line;
    int count = 0;
    std::vector<std::string> headers;

    while (std::getline(request_stream, line))
    {
        if (line.empty() && count == 0)
        {
            count++;
            continue;
        }
        else if (line.empty() && count == 1)
        {
            break;
        }

        if (count == 0)
            headers.push_back(line);
    }

    std::string boundary = headers.at(2).substr(headers.at(2).find("boundary=") + 9);
    size_t body_start = request.find("--" + boundary);
    if (body_start == std::string::npos)
    {
        //400 bad request
        std::cerr << "Error: Could not find body start." << std::endl;
        return;
    }
    size_t body_end = request.find("--" + boundary + "--", body_start);
    if (body_end == std::string::npos)
    {
        //400 bad request
        std::cerr << "Error: Could not find body end." << std::endl;
        return;
    }
    // should be +4 for "\r\n but since we only have "\n in a string we'll +3 here"
    std::string body = request.substr(body_start + boundary.length() + 3, body_end - body_start - boundary.length() - 3);

    size_t fn_pos = body.find("filename=\"");
    if (fn_pos == std::string::npos)
    {
        //no error maybe it's just a text not a file (username e.g)
        std::cerr << "Error: No filename found.\n";
        return;
    }
    fn_pos += 10;
    size_t fn_end = body.find('"', fn_pos);
    if (fn_end == std::string::npos)
    {
        //400 bad request
        std::cerr << "Error: Malformed filename.\n";
        return;
    }
    std::string filename = body.substr(fn_pos, fn_end - fn_pos);
    if(filename.empty())
        filename = generate_filename();
    std::fstream file(filename.c_str(), std::ios::out);
    if (!file)
    {
        //500 internal
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    // don't forget it's \r\n in real requests now we only working with \n\n
    size_t content_start = body.find("\n\n");
    if (content_start == std::string::npos)
    {
        //400 bad request (need to check if an upload request can be bodyless)
        std::cerr << "Error: Could not find content start." << std::endl;
        return;
    }
    content_start += 2; // Skip past the "\r\n\r\n"
    file.write(body.c_str() + content_start, body.size() - content_start);
    if (!file)
    {
        //internal server error
        std::cerr << "Error: Could not write to file " << filename << std::endl;
        return;
    }
    file.close();
}

void server()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        close(fd);
        return;
    }

    if (listen(fd, SOMAXCONN) == -1)
    {
        perror("listen");
        close(fd);
        return;
    }

    int client_fd = accept(fd, NULL, NULL);
    if (client_fd == -1)
    {
        perror("accept");
        return;
    }
    else
        std::cout << "Client connected " << client_fd << std::endl;

    // Handle client connection
    char buffer[1024];
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        // std::cout << "Received message from client " << client_fd << ": " << buffer << std::endl;
        // Process the received message
        handle_request(buffer);
    }
    close(client_fd);

    close(fd);
}
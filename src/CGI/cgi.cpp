#include "../../inc/webserv.hpp"

CGI::CGI()
{
    cgi_fd = -1;
    cgi_pid = -1;
    output_buffer = "";
    env_vars = std::map<std::string, std::string>();
    script_path = "";
    path_info = "";
    query_string = "";
    is_cgi = 0;
}

CGI::~CGI()
{
    close_cgi();
}

bool CGI::set_env_var(std::map<std::string, std::string>& env_vars, const ParsingRequest& request)
{
    env_vars["REQUEST_METHOD"] = request.getStartLine().at("method");
    env_vars["SCRIPT_NAME"] = script_path;
    env_vars["PATH_INFO"] = path_info;
    env_vars["QUERY_STRING"] = query_string;
    env_vars["HTTP_VERSION"] = request.getStartLine().at("version");
    env_vars["SERVER_PROTOCOL"] = request.getStartLine().at("version");
    if (request.getHeaders().find("host") != request.getHeaders().end())
        env_vars["SERVER_NAME"] = request.getHeaders().at("host");
    else
        env_vars["SERVER_NAME"] = "localhost";
    if (request.getHeaders().find("content-type") != request.getHeaders().end())
        env_vars["CONTENT_TYPE"] = request.getHeaders().at("content-type");
    else
        env_vars["CONTENT_TYPE"] = "";

    if (request.getHeaders().find("content-length") != request.getHeaders().end())
        env_vars["CONTENT_LENGTH"] = request.getHeaders().at("content-length");
    else
        env_vars["CONTENT_LENGTH"] = "0";

    if (request.getHeaders().find("user-agent") != request.getHeaders().end())
        env_vars["HTTP_USER_AGENT"] = request.getHeaders().at("user-agent");
    else
        env_vars["HTTP_USER_AGENT"] = "";

    env_vars["GATEWAY_INTERFACE"] = "CGI/1.1";
    env_vars["SERVER_SOFTWARE"] = "Webserv/1.0";
    // env_vars["REMOTE_ADDR"] = "127.0.0.1"; // i need to get this from the socket
    env_vars["REMOTE_HOST"] = "localhost";
    if (request.getHeaders().find("transfer-encoding") != request.getHeaders().end())
    {
        std::string transfer_encoding = request.getHeaders().at("transfer-encoding");
        if (transfer_encoding == "chunked")
        {
            // Note: The request parser should have already un-chunked the body
            // CGI should receive the complete body without chunk headers
            env_vars["TRANSFER_ENCODING"] = "";  // Clear for CGI
        }
    }
    if (!path_info.empty())
    {
        env_vars["PATH_TRANSLATED"] = "/www/html" + path_info;
    }
    else
    {
        env_vars["PATH_TRANSLATED"] = "";
    }

    return true;
}


bool CGI::execute(std::map<std::string, std::string>& env_vars)
{
    std::string full_script_path = "/www/html" + script_path; // this sould be getting from the config file
    if (access(full_script_path.c_str(), F_OK) != 0)
    {
        error_code = 404;
        error_message = "CGI script not found: " + full_script_path;
        std::cerr << "CGI script not found: " << full_script_path << std::endl;
        return false;
    }
    
    if (access(full_script_path.c_str(), X_OK) != 0)
    {
        error_code = 403;
        error_message = "CGI script not executable: " + full_script_path;
        std::cerr << "CGI script not executable: " << full_script_path << std::endl;
        return false;
    }

    int pipe_in[2];  // For sending data TO the CGI script
    int pipe_out[2]; // For receiving data FROM the CGI script

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        perror("pipe failed");
        return false;
    }

    std::vector<std::string> env_strings;
    std::vector<char*> envp;

    // Build environment variables
    for (std::map<std::string, std::string>::const_iterator it = env_vars.begin(); it != env_vars.end(); ++it)
    {
        std::string env_var = it->first + "=" + it->second;
        env_strings.push_back(env_var);
        envp.push_back(&env_strings.back()[0]);
    }
    envp.push_back(NULL);

    cgi_pid = fork();
    if (cgi_pid < 0)
    {
        perror("fork failed");
        error_code = 500;
        error_message = "Internal server error";
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        return false;
    }

    if (cgi_pid == 0)
    {
        close(pipe_in[1]);
        close(pipe_out[0]);

        if (dup2(pipe_in[0], STDIN_FILENO) == -1 ||
            dup2(pipe_out[1], STDOUT_FILENO) == -1)
        {
            perror("dup2 failed in child");
            error_code = 500;
            error_message = "Internal server error";
            exit(EXIT_FAILURE);
        }
        close(pipe_in[0]);
        close(pipe_out[1]);

        std::vector<char*> argv;
        std::string interpreter = get_interpreter(full_script_path);
        
        if (!interpreter.empty())
        {
            argv.push_back(const_cast<char*>(interpreter.c_str()));
            argv.push_back(const_cast<char*>(full_script_path.c_str()));
        }
        else
        {
            argv.push_back(const_cast<char*>(full_script_path.c_str()));
        }
        argv.push_back(NULL);

        std::string script_dir = full_script_path.substr(0, full_script_path.find_last_of('/'));
        if (chdir(script_dir.c_str()) != 0)
        {
            perror("chdir failed");
        }
        if (!interpreter.empty())
        {
            execve(interpreter.c_str(), &argv[0], &envp[0]);
        }
        else
        {
            execve(full_script_path.c_str(), &argv[0], &envp[0]);
        }
        
        perror("execve failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);
        int input_fd = pipe_in[1];
        cgi_fd = pipe_out[0];

        close(input_fd);

        bool process_finished = wait_with_timeout(5);
        if (!process_finished)
        {
            logError("CGI script timeout");
            error_code = 504;
            error_message = "Gateway Timeout";
            kill(cgi_pid, SIGKILL);
            waitpid(cgi_pid, &status, 0);
            close(cgi_fd);
            cgi_fd = -1;
            return false;
        }

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            std::cerr << "CGI script failed with status: " << WEXITSTATUS(status) << std::endl;
            close(cgi_fd);
            cgi_fd = -1;
            error_code = 500;
            error_message = "Internal Server Error";
            return false;
        }
    }

    return true;
}

// use excute with body for POST requests


// bool CGI::execute_with_body(std::map<std::string, std::string>& env_vars, const std::string& body_data)
// {
//     // Validate script existence first
//     std::string full_script_path = "/var/www/html" + script_path;
//     if (access(full_script_path.c_str(), F_OK) != 0)
//     {
//         std::cerr << "CGI script not found: " << full_script_path << std::endl;
//         return false;
//     }
    
//     if (access(full_script_path.c_str(), X_OK) != 0)
//     {
//         std::cerr << "CGI script not executable: " << full_script_path << std::endl;
//         return false;
//     }

//     int pipe_in[2];
//     int pipe_out[2];

//     if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
//     {
//         perror("pipe failed");
//         return false;
//     }

//     std::vector<std::string> env_strings;
//     std::vector<char*> envp;

//     for (std::map<std::string, std::string>::const_iterator it = env_vars.begin(); it != env_vars.end(); ++it)
//     {
//         std::string env_var = it->first + "=" + it->second;
//         env_strings.push_back(env_var);
//         envp.push_back(&env_strings.back()[0]);
//     }
//     envp.push_back(NULL);

//     cgi_pid = fork();
//     if (cgi_pid < 0)
//     {
//         perror("fork failed");
//         close(pipe_in[0]);
//         close(pipe_in[1]);
//         close(pipe_out[0]);
//         close(pipe_out[1]);
//         return false;
//     }

//     if (cgi_pid == 0)
//     {
//         // Child process
//         close(pipe_in[1]);
//         close(pipe_out[0]);

//         if (dup2(pipe_in[0], STDIN_FILENO) == -1 ||
//             dup2(pipe_out[1], STDOUT_FILENO) == -1)
//         {
//             perror("dup2 failed in child");
//             exit(EXIT_FAILURE);
//         }

//         close(pipe_in[0]);
//         close(pipe_out[1]);

//         std::vector<char*> argv;
//         std::string interpreter = get_interpreter(full_script_path);
        
//         if (!interpreter.empty())
//         {
//             argv.push_back(const_cast<char*>(interpreter.c_str()));
//             argv.push_back(const_cast<char*>(full_script_path.c_str()));
//         }
//         else
//         {
//             argv.push_back(const_cast<char*>(full_script_path.c_str()));
//         }
//         argv.push_back(NULL);

//         if (!interpreter.empty())
//         {
//             execve(interpreter.c_str(), &argv[0], &envp[0]);
//         }
//         else
//         {
//             execve(full_script_path.c_str(), &argv[0], &envp[0]);
//         }
        
//         perror("execve failed");
//         exit(EXIT_FAILURE);
//     }
//     else
//     {
//         // Parent process
//         close(pipe_in[0]);
//         close(pipe_out[1]);

//         int input_fd = pipe_in[1];
//         cgi_fd = pipe_out[0];

//         // Send POST data
//         if (!body_data.empty())
//         {
//             if (!send_post_data(input_fd, body_data))
//             {
//                 close(input_fd);
//                 close(cgi_fd);
//                 kill(cgi_pid, SIGKILL);
//                 waitpid(cgi_pid, NULL, 0);
//                 return false;
//             }
//         }
//         close(input_fd);

//         // // Wait for CGI process with timeout
//         // bool process_finished = wait_with_timeout(10); // 10 second timeout for POST
        
//         // if (!process_finished)
//         // {
//         //     std::cerr << "CGI script timeout" << std::endl;
//         //     kill(cgi_pid, SIGKILL);
//         //     waitpid(cgi_pid, &status, 0);
//         //     close(cgi_fd);
//         //     cgi_fd = -1;
//         //     return false;
//         // }

//         if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
//         {
//             std::cerr << "CGI script failed with status: " << WEXITSTATUS(status) << std::endl;
//             close(cgi_fd);
//             cgi_fd = -1;
//             return false;
//         }
//     }

//     return true;
// }

bool CGI::read_output()
{
    if (cgi_fd < 0)
    {
        std::cerr << "Invalid CGI file descriptor" << std::endl;
        return false;
    }

    char buffer[4096];
    ssize_t bytes_read;
    output_buffer.clear();

    int flags = fcntl(cgi_fd, F_GETFL, 0);
    fcntl(cgi_fd, F_SETFL, flags | O_NONBLOCK);

    const int READ_TIMEOUT = 10;
    int total_intervals = READ_TIMEOUT * 20;
    bool data_received = false;

    for (int i = 0; i < total_intervals; ++i)
    {
        bytes_read = read(cgi_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            output_buffer += buffer;
            data_received = true;
            // Reset interval counter when we receive data
            i = 0; 
        }
        else if (bytes_read == 0)
        {
            // EOF reached , CGI is done done sf
            break;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // No data available, wait briefly before retrying
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 50000; // 50ms
            select(0, NULL, NULL, NULL, &tv);
            continue;
        }
        else
        {
            perror("read CGI output failed");
            fcntl(cgi_fd, F_SETFL, flags);
            return false;
        }
    }

    // Restore blocking mode
    fcntl(cgi_fd, F_SETFL, flags);

    if (output_buffer.empty() && !data_received)
    {
        error_code = 500;
        error_message = "No output received from CGI script";
        std::cerr << "No output received from CGI script" << std::endl;
        return false;
    }

    return true;
}

void CGI::close_cgi()
{
    if (cgi_fd >= 0)
    {
        close(cgi_fd);
        cgi_fd = -1;
    }

    if (cgi_pid > 0)
    {
        kill(cgi_pid, SIGTERM);
        waitpid(cgi_pid, NULL, 0);
        cgi_pid = -1;
    }
}

std::string CGI::get_interpreter(const std::string& script_path)
{
    if (script_path.find(".py") != std::string::npos)
        return "/usr/bin/python3";
    else if (script_path.find(".pl") != std::string::npos)
        return "/usr/bin/perl";
    else if (script_path.find(".php") != std::string::npos)
        return "/usr/bin/php";
    else if (script_path.find(".sh") != std::string::npos)
        return "/bin/bash";

    //if the file ends with a .cgi or its a binary
    // we need to check shebang eg : #!/bin/bash
    std::ifstream file(script_path.c_str());
    if (file.is_open())
    {
        std::string first_line;
        std::getline(file, first_line);
        file.close();
        
        if (first_line.length() > 2 && first_line.substr(0, 2) == "#!")
        {
            std::string shebang = first_line.substr(2);
            size_t end = shebang.find_first_of(" \t\r\n");
            if (end != std::string::npos)
                shebang = shebang.substr(0, end);
            return shebang;
        }
    }
    
    return "";
}

bool CGI::wait_with_timeout(int timeout_seconds)
{
    int wait_status;
    
    int total_intervals = timeout_seconds * 10; // Check every 100ms
    
    for (int i = 0; i < total_intervals; ++i)
    {
        pid_t result = waitpid(cgi_pid, &wait_status, WNOHANG);
        
        if (result == cgi_pid)
        {
            status = wait_status;
            return true;
        }
        else if (result == -1)
        {
            perror("waitpid failed");
            return false;
        }
        
        // Process still running
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms
        select(0, NULL, NULL, NULL, &tv);
    }
    
    return false; // Timeout
}

// bool CGI::send_post_data(int fd, const std::string& body_data)
// {
//     if (body_data.empty())
//         return true;
        
//     size_t total_sent = 0;
//     size_t data_size = body_data.length();
    
//     while (total_sent < data_size)
//     {
//         ssize_t sent = write(fd, body_data.c_str() + total_sent, data_size - total_sent);
//         if (sent <= 0)
//         {
//             perror("Failed to send POST data to CGI");
//             return false;
//         }
//         total_sent += sent;
//     }
    
//     return true;
// }


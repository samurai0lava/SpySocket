#include "../../inc/webserv.hpp"


CGI::CGI()
{
    cgi_fd = -1;
    cgi_pid = -1;
    output_buffer = "";
    env_vars = std::map<std::string, std::string>();
    script_path = "";
}

CGI::~CGI()
{
    close_cgi();
}

bool CGI::set_env_var(std::map<std::string, std::string>& env_vars, const ParsingRequest& request)
{
    // Required CGI environment variables
    env_vars["REQUEST_METHOD"] = request.getStartLine().at("method");
    env_vars["SCRIPT_NAME"] = request.getStartLine().at("uri");
    env_vars["HTTP_VERSION"] = request.getStartLine().at("version");
    env_vars["SERVER_PROTOCOL"] = request.getStartLine().at("version");
    env_vars["SERVER_NAME"] = request.getHeaders().at("host");

    // Query string (extract from URI)
    std::string uri = request.getStartLine().at("uri");
    size_t query_pos = uri.find('?');
    if (query_pos != std::string::npos)
    {
        env_vars["QUERY_STRING"] = uri.substr(query_pos + 1);
        env_vars["SCRIPT_NAME"] = uri.substr(0, query_pos); // Update script name without query
    }
    else
    {
        env_vars["QUERY_STRING"] = "";
    }
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

    // Additional standard CGI variables
    // env_vars["GATEWAY_INTERFACE"] = "CGI/1.1";
    // env_vars["SERVER_SOFTWARE"] = "Webserv/1.0";
    // env_vars["REMOTE_ADDR"] = "127.0.0.1"; //get this from the socket
    // env_vars["REMOTE_HOST"] = "localhost";
    // env_vars["PATH_INFO"] = "";
    // env_vars["PATH_TRANSLATED"] = "";

    return true;
}


bool CGI::execute(std::map<std::string, std::string>& env_vars)
{
    int pipe_in[2];  // For sending data TO the CGI script
    int pipe_out[2]; // For receiving data FROM the CGI script

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        perror("pipe failed");
        return false;
    }

    std::vector<std::string> env_strings;
    std::vector<char*> envp;

    for (const std::map<std::string, std::string>::iterator it = env_vars.begin(); it != env_vars.end(); ++it)
    {
        std::string env_var = it->first + "=" + it->second;
        env_strings.push_back(env_var);
        envp.push_back(&env_strings.back()[0]);
    }
    envp.push_back(NULL);

    // //print the environment variables
    // std::cout << "CGI Environment Variables:" << std::endl;
    // for (std::map<std::string, std::string>::const_iterator it = env_vars.begin(); it != env_vars.end(); ++it)
    // {
    //     std::cout << it->first << "=" << it->second << std::endl;
    // }

    // Create the CGI process
    cgi_pid = fork();
    if (cgi_pid < 0)
    {
        perror("fork failed");
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        return false;
    }

    //CHILD PROCESS
    if (cgi_pid == 0)
    {
        close(pipe_in[1]);
        close(pipe_out[0]);

        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[0]);
        close(pipe_out[1]);
        script_path = env_vars["SCRIPT_NAME"];
        if (script_path.empty())
        {
            std::cerr << "CGI script path is empty" << std::endl;
            exit(EXIT_FAILURE);
        }

        char* argv[2];
        argv[0] = const_cast<char*>(script_path.c_str());
        argv[1] = NULL;
        execve(script_path.c_str(), argv, &envp[0]);
        perror("execve failed");
        exit(EXIT_FAILURE);
    }
    //PARENT PROCESS
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);
        cgi_fd = pipe_out[0];
        close(pipe_in[1]);
        waitpid(cgi_pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            close(cgi_fd);
            return false;
        }
    }

    return true;
}

bool CGI::read_output()
{
    if (cgi_fd < 0)
        return false;

    char buffer[4096];
    ssize_t bytes_read;
    output_buffer.clear();

    while ((bytes_read = read(cgi_fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        output_buffer += buffer;
    }

    if (bytes_read < 0)
    {
        perror("read CGI output failed");
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

bool CGI::check_is_cgi(const ParsingRequest& request)
{

    //check the uri is the cgi-bin directory
    std::string uri = request.getStartLine().at("uri");
    
    //we Need to decode this uri







}

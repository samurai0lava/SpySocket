#include "../../inc/webserv.hpp"

CGI::CGI()
{
    cgi_fd = -1;
    cgi_pid = -1;
    cgi_start_time = 0;
    output_buffer = "";
    env_vars = std::map<std::string, std::string>();
    script_path = "";
    path_info = "";
    query_string = "";
    is_cgi = 0;
    error_code = 0;
    error_message = "";
}

CGI::~CGI()
{
    close_cgi();
}

void CGI::set_location(const LocationStruct& location)
{
    this->current_location = location;
}

bool CGI::set_env_var(std::map<std::string, std::string>& env_vars, const ParsingRequest& request)
{
    this->env_vars.clear();


    std::map<std::string, std::string> startLine = request.getStartLine();
    std::map<std::string, std::string> headers = request.getHeaders();

    if (startLine.find("method") != startLine.end())
        this->env_vars["REQUEST_METHOD"] = startLine.at("method");
    else
        this->env_vars["REQUEST_METHOD"] = "GET";

    this->env_vars["SCRIPT_NAME"] = script_path;
    this->env_vars["PATH_INFO"] = path_info;
    this->env_vars["QUERY_STRING"] = query_string;

    if (startLine.find("version") != startLine.end()) {
        this->env_vars["HTTP_VERSION"] = startLine.at("version");
        this->env_vars["SERVER_PROTOCOL"] = startLine.at("version");
    }
    else {
        this->env_vars["HTTP_VERSION"] = "HTTP/1.1";
        this->env_vars["SERVER_PROTOCOL"] = "HTTP/1.1";
    }

    if (headers.find("host") != headers.end())
        this->env_vars["SERVER_NAME"] = headers.at("host_name");
    else
        this->env_vars["SERVER_NAME"] = "localhost";

    if (headers.find("content-type") != headers.end())
        this->env_vars["CONTENT_TYPE"] = headers.at("content-type");
    else
        this->env_vars["CONTENT_TYPE"] = "";

    if (headers.find("content-length") != headers.end())
        this->env_vars["CONTENT_LENGTH"] = headers.at("content-length");
    else
        this->env_vars["CONTENT_LENGTH"] = "0";

    if (headers.find("user-agent") != headers.end())
        this->env_vars["HTTP_USER_AGENT"] = headers.at("user-agent");
    else
        this->env_vars["HTTP_USER_AGENT"] = "";

    this->env_vars["GATEWAY_INTERFACE"] = "CGI/1.1";
    this->env_vars["SERVER_SOFTWARE"] = "SpySocket/1.0";
    this->env_vars["REMOTE_HOST"] = headers.at("host");
    this->env_vars["SERVER_PORT"] = headers.at("port");

    if (headers.find("transfer-encoding") != headers.end())
    {
        std::string transfer_encoding = headers.at("transfer-encoding");
        if (transfer_encoding == "chunked")
        {
            this->env_vars["TRANSFER_ENCODING"] = "";
        }
    }

    if (!path_info.empty())
    {
        this->env_vars["PATH_TRANSLATED"] = this->current_location.root + path_info;
    }
    else
    {
        this->env_vars["PATH_TRANSLATED"] = "";
    }

    env_vars = this->env_vars;
    return true;
}

bool CGI::execute(std::map<std::string, std::string>& env_vars, const LocationStruct& location)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    std::string full_script_path = std::string(cwd) + "/" + this->current_location.root + script_path;

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

    int pipe_in[2];
    int pipe_out[2];

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        perror("pipe failed");
        return false;
    }

    std::vector<std::string> env_strings;
    std::vector<char*> envp;

    for (std::map<std::string, std::string>::const_iterator it = env_vars.begin(); it != env_vars.end(); ++it)
    {
        std::string env_var = it->first + "=" + it->second;
        env_strings.push_back(env_var);
    }

    for (size_t i = 0; i < env_strings.size(); ++i) {
        envp.push_back(&env_strings[i][0]);
    }
    envp.push_back(NULL);

    cgi_pid = fork();
    if (cgi_pid < 0)
    {
        perror("fork failed");
        error_code = 500;
        error_message = "Fork Failed : Internal server error";
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

        // Redirect stderr to stdout so all output goes through the pipe
        dup2(STDOUT_FILENO, STDERR_FILENO);

        close(pipe_in[0]);
        close(pipe_out[1]);

        std::vector<char*> argv;
        std::string interpreter = get_interpreter(full_script_path, location);

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
        cgi_start_time = std::time(NULL);
        int flags = fcntl(cgi_fd, F_GETFL, 0);
        if (flags != -1) {
            fcntl(cgi_fd, F_SETFL, flags | O_NONBLOCK);
        }

        close(input_fd);
    }

    return true;
}

bool CGI::execute_with_body(std::map<std::string, std::string>& env_vars, const std::string& body_data, const LocationStruct& location)
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    std::string full_script_path = std::string(cwd) + "/" + this->current_location.root + script_path;
    std::cout << RED << full_script_path << RESET << std::endl;

    if (access(full_script_path.c_str(), F_OK) != 0) {
        error_code = 404;
        error_message = "CGI script not found: " + full_script_path;
        access_error(error_code, error_message);
        std::cerr << "CGI script not found: " << full_script_path << std::endl;
        return false;
    }

    if (access(full_script_path.c_str(), X_OK) != 0) {
        error_code = 403;
        error_message = "CGI script not executable: " + full_script_path;
        access_error(error_code, error_message);
        std::cerr << "CGI script not executable: " << full_script_path << std::endl;
        return false;
    }

    int pipe_in[2];
    int pipe_out[2];

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        access_error(500, "Internal Server Error: Pipe creation failed");
        perror("pipe failed");
        return false;
    }

    std::vector<std::string> env_strings;
    std::vector<char*> envp;

    for (std::map<std::string, std::string>::const_iterator it = env_vars.begin(); it != env_vars.end(); ++it) {
        std::string env_var = it->first + "=" + it->second;
        env_strings.push_back(env_var);
    }
    for (size_t i = 0; i < env_strings.size(); ++i) {
        envp.push_back(&env_strings[i][0]);
    }
    envp.push_back(NULL);

    cgi_pid = fork();
    if (cgi_pid < 0) {
        perror("fork failed");
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        return false;
    }

    if (cgi_pid == 0) {
        // Child process
        close(pipe_in[1]);
        close(pipe_out[0]);

        if (dup2(pipe_in[0], STDIN_FILENO) == -1 ||
            dup2(pipe_out[1], STDOUT_FILENO) == -1) {
            perror("dup2 failed in child");
            exit(EXIT_FAILURE);
        }

        // Redirect stderr to stdout so all output goes through the pipe
        dup2(STDOUT_FILENO, STDERR_FILENO);

        close(pipe_in[0]);
        close(pipe_out[1]);
        std::string script_dir = full_script_path.substr(0, full_script_path.find_last_of('/'));
        std::cerr.flush();
        if (chdir(script_dir.c_str()) != 0) {
            perror("chdir failed");
        }

        std::vector<char*> argv;
        std::string interpreter = get_interpreter(full_script_path, location);
        std::cerr.flush();

        if (!interpreter.empty()) {
            argv.push_back(const_cast<char*>(interpreter.c_str()));
            argv.push_back(const_cast<char*>(full_script_path.c_str()));
        }
        else {
            argv.push_back(const_cast<char*>(full_script_path.c_str()));
        }
        argv.push_back(NULL);

        if (!interpreter.empty()) {
            execve(interpreter.c_str(), &argv[0], &envp[0]);
        }
        else {
            execve(full_script_path.c_str(), &argv[0], &envp[0]);
        }

        access_error(500, "Internal Server Error: execve failed");
        perror("execve failed");
        exit(EXIT_FAILURE);
    }
    else {
        close(pipe_in[0]);
        close(pipe_out[1]);

        int input_fd = pipe_in[1];
        cgi_fd = pipe_out[0];
        cgi_start_time = std::time(NULL);

        int flags = fcntl(cgi_fd, F_GETFL, 0);
        if (flags != -1) {
            fcntl(cgi_fd, F_SETFL, flags | O_NONBLOCK);
        }
        if (!body_data.empty())
        {
            if (!send_post_data(input_fd, body_data)) {
                close(input_fd);
                close(cgi_fd);
                kill(cgi_pid, SIGKILL);
                waitpid(cgi_pid, NULL, 0);
                return false;
            }
        }
        close(input_fd);
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

    cgi_start_time = 0;
}


std::string CGI::get_interpreter(const std::string& script_path, const LocationStruct& location)
{
    std::string extension = "";
    size_t dot_pos = script_path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        extension = script_path.substr(dot_pos);
    }

    if (!location.cgi_path.empty() && !location.cgi_ext.empty()) {
        for (size_t i = 0; i < location.cgi_ext.size(); ++i) {
            if (extension == location.cgi_ext[i]) {
                if (i < location.cgi_path.size()) {
                    if (access(location.cgi_path[i].c_str(), X_OK) == 0) {
                        return location.cgi_path[i];
                    }
                    error_code = 500;
                    error_message = "Configured CGI interpreter not executable";
					access_error(error_code, error_message);
                    return "";
                }
            }
        }
    }

    std::string interpreter = "";

    if (extension == ".py") {
        interpreter = "/usr/bin/python3";
    }
    else if (extension == ".pl") {
        interpreter = "/usr/bin/perl";
    }
    else if (extension == ".php") {
        interpreter = "/usr/bin/php";
    }
    else if (extension == ".sh") {
        interpreter = "/bin/bash";
    }
    if (!interpreter.empty()) {
        if (access(interpreter.c_str(), X_OK) == 0) {
            return interpreter;
        }
        error_code = 500;
        error_message = "Default CGI interpreter not found or not executable";
		access_error(error_code, error_message);
        return "";
    }

    std::ifstream file(script_path.c_str());
    if (file.is_open())
    {
        std::string first_line;
        std::getline(file, first_line);
        file.close();

        if (first_line.length() > 2 && first_line.substr(0, 2) == "#!")
        {
            std::string shebang = first_line.substr(2);
            size_t start = shebang.find_first_not_of(" \t");
            if (start != std::string::npos) {
                shebang = shebang.substr(start);
            }

            size_t end = shebang.find_first_of(" \t\r\n");
            if (end != std::string::npos) {
                shebang = shebang.substr(0, end);
            }
            if (!shebang.empty() && access(shebang.c_str(), X_OK) == 0) {
                return shebang;
            }

            error_code = 500;
            error_message = "Shebang interpreter not found or not executable";
			access_error(error_code, error_message);
            return "";
        }
    }

    error_code = 500;
    error_message = "Could not determine CGI interpreter";
	access_error(error_code, error_message);
    return "";
}


bool CGI::send_post_data(int fd, const std::string& body_data)
{
    if (body_data.empty()) {
        return true;
    }

    size_t total_sent = 0;
    size_t data_size = body_data.length();


    while (total_sent < data_size)
    {
        ssize_t sent = write(fd, body_data.c_str() + total_sent, data_size - total_sent);
        if (sent <= 0)
        {
            access_error(500, "Internal Server Error: Failed to send POST data to CGI");
            perror("Failed to send POST data to CGI");
            return false;
        }
        total_sent += sent;
    }

    return true;
}
bool CGI::is_cgi_timeout(int timeout_seconds)
{
    if (cgi_pid <= 0 || cgi_start_time == 0)
        return false;

    time_t current_time = std::time(NULL);
    return (current_time - cgi_start_time) >= timeout_seconds;
}
bool CGI::read_output()
{
    if (cgi_fd < 0) {
        return false;
    }

    char buffer[8192];
    ssize_t bytes_read = read(cgi_fd, buffer, sizeof(buffer) - 1);


    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        output_buffer.append(buffer, bytes_read);
        return true;
    }
    else if (bytes_read == 0)
    {
        close(cgi_fd);
        cgi_fd = -1;
        return false;
    }
    else if (bytes_read == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return false;
        }
        else
        {
            perror("Failed to read from CGI process");
            close(cgi_fd);
            cgi_fd = -1;
            return false;
        }
    }

    return false;
}

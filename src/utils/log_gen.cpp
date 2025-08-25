#include "../../inc/webserv.hpp"

std::string generate_access_line(ParsingRequest& request)
{
    std::string access_line;
    std::ostringstream oss;

    oss << "[";
    oss << ft_time_format();
    oss << "] ";
    oss << request.getStartLine().at("method");
    oss << " ";
    oss << request.getStartLine().at("uri");
    oss << " ";
    oss << request.getStartLine().at("version");
    oss << " ";
    oss << request.getHeaders().at("host");
    oss << " ";
    if(request.getHeaders().count("user-agent"))
    {
        oss << request.getHeaders().at("user-agent");
        oss << " ";
    }
    if (request.getHeaders().count("connection"))
    {
        oss << request.getHeaders().at("connection");
        oss << " ";
    }
    
    oss << "\n";
    access_line = oss.str();

    return access_line;
}

std::string generate_error_line(int error_code, const std::string& error_message)
{
    std::string error_line;
    std::ostringstream oss;
    std::string time = ft_time_format();
    std::cout << time << std::endl;

    oss << "[";
    oss << time;
    oss << "] ";
    oss << "Error: ";
    oss << error_code;
    oss << " - ";
    oss << error_message;
    oss << "\n";

    error_line = oss.str();

    return error_line;
}

void access_log(ParsingRequest& request)
{
    std::string access_message;
    access_message = generate_access_line(request);
    std::ofstream access_log_file("log/access.log", std::ios::app);
    if (!access_log_file)
    {
        std::cerr << "Failed to open access log file" << std::endl;
        return;
    }
    access_log_file << access_message;
    access_log_file.close();
}


void access_error(int error_code, const std::string& error_message)
{
    std::string error_log_message;
    std::ofstream error_log_file("log/error.log", std::ios::app);
    if (!error_log_file)
    {
        std::cerr << "Failed to open error log file" << std::endl;
        return ;
    }
    error_log_message = generate_error_line(error_code, error_message);
    error_log_file << error_log_message;
    error_log_file.close();
}


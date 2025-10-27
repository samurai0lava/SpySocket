#include "../../../inc/webserv.hpp"

void printMap(const std::map<std::string, std::string>& m)
{
    for (std::map<std::string, std::string>::const_iterator it = m.begin(); it != m.end(); ++it)
    {
        std::cout << "      " << YELLOW << it->first << " --> " RESET << it->second << std::endl;
    }
}

void logError(int status, const std::string& error_message)
{
    std::cerr << RED "Error " << status << ": " << error_message << RESET << std::endl;
}

bool ParsingRequest::find_crlf(size_t& pos)
{
    pos = buffer.find("\r\n", buffer_pos);
    return (pos != std::string::npos);
}

ParsingRequest::ParseResult ParsingRequest::get_parse_status() const
{
    switch (current_state)
    {
    case PARSE_COMPLETE:
        return PARSE_OK;
    case PARSE_ERROR:
        return PARSE_ERROR_400;
    default:
        return PARSE_AGAIN;
    }
}


void printRequestInfo(const ParsingRequest& request, int fd)
{
    std::cout << BLUE "Request parsed successfully on fd " RESET << fd << std::endl;
    std::cout << std::endl;
    std::cout << GREEN "Received: " RESET << std::endl;
    printMap(request.getStartLine());
    printMap(request.getHeaders());
    std::cout << request.getBody() << std::endl;
    // std::cout << "----------------------------------------------" << std::endl;
    // std::cout << BLUE "Host exists: " RESET << (request.getHostExists() ? "Yes" : "No") << std::endl;
    // std::cout << BLUE "Content-Length exists: " RESET << (request.getContentLengthExists() ? "Yes" : "No") << std::endl;
    // std::cout << BLUE "Transfer-Encoding exists: " RESET << (request.getTransferEncodingExists() ? "Yes" : "No") << std::endl;
    // std::cout << BLUE "Connection status: " RESET << (request.getConnectionStatus() ? "Keep-Alive" : "Closed") << std::endl;
    // std::cout << "----------------------------------------------" << std::endl;
}

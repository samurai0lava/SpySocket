#include "../inc/webserv.hpp"

CClient::CClient() :
    _name_location(""), NameMethod(""), uri(""), FdClient(-1), mutableConfig(), serv(), parser(NULL),
    SendHeader(false), readyToSendAllResponse(false), chunkedSending(false),
    chunkSize(0), bytesSent(0), response(""), filePath(""), fileSize(0),
    offset(0), fileFd(-1), intialized(false), Chunked(false),
    cgi_handler(NULL), is_cgi_request(false), cgi_headers_sent(false), cgi_body_buffer(""),
    should_close_connection(false)
{

}

CClient::CClient(std::string NameMethod, std::string uri, int FdClient, ConfigStruct MConfig, Servers* serv, ParsingRequest* parser) :
    _name_location(""), NameMethod(NameMethod), uri(uri), FdClient(FdClient), mutableConfig(MConfig), serv(serv), parser(parser),
    SendHeader(false), readyToSendAllResponse(false), chunkedSending(false), chunkSize(0), bytesSent(0),
    response(""), filePath(""), fileSize(0), offset(0), fileFd(-1), intialized(false), Chunked(false),
    cgi_handler(NULL), is_cgi_request(false), cgi_headers_sent(false), cgi_body_buffer(""),
    should_close_connection(false)

{

}

CClient::~CClient()
{
    if (cgi_handler) {
        cgi_handler->close_cgi();
        delete cgi_handler;
        cgi_handler = NULL;
    }
}
void CClient::printInfo() const {
    std::cout << "===== Client Info =====" << std::endl;
    std::cout << "Method: " << NameMethod << std::endl;
    std::cout << "URI: " << uri << std::endl;
    std::cout << "FdClient: " << FdClient << std::endl;
    // std::cout << "Read Buffer: " << readbuf << std::endl;
    // std::cout << "Offset: " << offset << std::endl;
    // std::cout << "Size File: " << sizeFile << std::endl;
    std::cout << "Parser ptr: " << parser << std::endl;
    std::cout << "Config root: " << mutableConfig.root << std::endl;
    std::cout << "Config indexPage: " << mutableConfig.indexPage << std::endl;

    std::cout << "=======================" << std::endl;
}


std::string CClient::HandleAllMethod()
{

    if (is_cgi_request && cgi_handler) {
        return HandleCGIMethod();
    }

    if (!cgi_handler) {
        cgi_handler = new CGI();
    }

    if (!is_cgi_request)
    {
        if (cgi_handler->check_is_cgi(*parser))
        {
            is_cgi_request = true;
            std::pair<std::string, LocationStruct> loc_pair = get_location(uri, mutableConfig);
            LocationStruct location = loc_pair.second;

            std::map<std::string, std::string> env_vars;
            if (!cgi_handler->set_env_var(env_vars, *parser)) {
                cgi_handler->close_cgi();
                delete cgi_handler;
                cgi_handler = NULL;
                is_cgi_request = false;
                return getErrorPageFromConfig(500);
            }

            bool success = false;
            if (this->NameMethod == "POST") {
                success = cgi_handler->execute_with_body(env_vars, parser->getBody(), location);
            }
            else {
                success = cgi_handler->execute(env_vars, location);
            }

            if (!success) {
                int error_code = cgi_handler->get_error_code();
                cgi_handler->close_cgi();
                delete cgi_handler;
                cgi_handler = NULL;
                is_cgi_request = false;
                return getErrorPageFromConfig(error_code > 0 ? error_code : 500);
            }
            return "";
        }
    }

    if (!is_cgi_request)
    {
        if (cgi_handler) {
            cgi_handler->close_cgi();
            delete cgi_handler;
            cgi_handler = NULL;
        }
    }
    if (this->NameMethod == "GET")
    {
        Get _MGet(*this);
        try {
            return (_MGet.MethodGet());
        }
        catch (const std::runtime_error& e) {
            std::string errMsg = e.what();
            if (errMsg.find("405") != std::string::npos) {
                return getErrorPageFromConfig(405);
            }
            else if (errMsg.find("403") != std::string::npos) {
                return getErrorPageFromConfig(403);
            }
            else {
                return getErrorPageFromConfig(500);
            }
        }
    }
    else if (this->NameMethod == "DELETE")
    {
        DeleteMethode MDelete;
        return (MDelete.PerformDelete(this->uri, this->mutableConfig));
    }
    else if (this->NameMethod == "POST")
    {
        return postMethod(this->uri, this->mutableConfig, *this->parser);
    }
    else
        return getErrorPageFromConfig(405);
    return std::string();
}

std::string CClient::HandleCGIMethod()
{
    if (!cgi_handler) {
        return getErrorPageFromConfig(500);
    }

    int status;
    pid_t cgi_pid = cgi_handler->get_cgi_pid();
    pid_t result = waitpid(cgi_pid, &status, WNOHANG);
    std::string current_output = cgi_handler->get_output_buffer();

    if (result == -1) {
        cgi_handler->close_cgi();
        delete cgi_handler;
        cgi_handler = NULL;
        is_cgi_request = false;
        return getErrorPageFromConfig(500);
    }
    else if (result == cgi_pid) {
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            cgi_handler->close_cgi();
            delete cgi_handler;
            cgi_handler = NULL;
            is_cgi_request = false;
            return getErrorPageFromConfig(502); // Bad Gateway
        }
        int read_attempts = 0;
        while (cgi_handler->read_output() && read_attempts < 10) {
            read_attempts++;
        }
        std::string cgi_output = cgi_handler->get_output_buffer();
        cgi_handler->close_cgi();
        delete cgi_handler;
        cgi_handler = NULL;
        is_cgi_request = false;
        return formatCGIResponse(cgi_output);
    }
    else {
        if (cgi_handler->is_cgi_timeout(CGI_TIMEOUT)) {
            cgi_handler->close_cgi();
            delete cgi_handler;
            cgi_handler = NULL;
            is_cgi_request = false;
            return getErrorPageFromConfig(504);
        }

        int read_count = 0;
        while (cgi_handler->read_output() && read_count < 100) {
            read_count++;
        }

        current_output = cgi_handler->get_output_buffer();
        int cgi_fd = cgi_handler->get_cgi_fd();

        usleep(1000);
        result = waitpid(cgi_pid, &status, WNOHANG);
        if (result == cgi_pid && cgi_fd >= 0) {
            int drain_attempts = 0;
            while (cgi_handler->read_output() && drain_attempts < 100) {
                drain_attempts++;
                usleep(100);
            }
            current_output = cgi_handler->get_output_buffer();
            cgi_fd = cgi_handler->get_cgi_fd();
        }
        if (result == cgi_pid) {
            cgi_handler->close_cgi();
            std::string cgi_output = current_output;
            delete cgi_handler;
            cgi_handler = NULL;
            is_cgi_request = false;
            return formatCGIResponse(cgi_output);
        }

        if (cgi_fd < 0 && !current_output.empty()) {
            cgi_handler->close_cgi();
            std::string cgi_output = current_output;
            delete cgi_handler;
            cgi_handler = NULL;
            is_cgi_request = false;
            return formatCGIResponse(cgi_output);
        }
        if (!current_output.empty() && current_output.find("Content-Type:") != std::string::npos) {
            size_t header_end = current_output.find("\r\n\r\n");
            if (header_end == std::string::npos) {
                header_end = current_output.find("\n\n");
            }
            if (header_end != std::string::npos) {
                std::string content_type_line = current_output.substr(0, current_output.find("\n"));
                if (content_type_line.find("text/html") != std::string::npos) {
                    if (current_output.find("</html>") != std::string::npos ||
                        current_output.find("<!DOCTYPE html>") != std::string::npos) {
                        cgi_handler->close_cgi();
                        std::string cgi_output = current_output;
                        delete cgi_handler;
                        cgi_handler = NULL;
                        is_cgi_request = false;
                        return formatCGIResponse(cgi_output);
                    }
                }
            }
        }
        return "";
    }
}

std::string CClient::formatCGIResponse(const std::string& cgi_output)
{
    if (cgi_output.empty()) {
        return getErrorPageFromConfig(500);
    }


    size_t headers_end = cgi_output.find("\r\n\r\n");
    if (headers_end == std::string::npos) {
        headers_end = cgi_output.find("\n\n");
        if (headers_end == std::string::npos) {
            return getErrorPageFromConfig(500);
        }
        headers_end += 2;
    }
    else {
        headers_end += 4;
    }

    std::string cgi_headers = cgi_output.substr(0, headers_end);
    std::string cgi_body = cgi_output.substr(headers_end);


    std::string response = "HTTP/1.1 200 OK\r\n";
    std::istringstream header_stream(cgi_headers);
    std::string line;
    bool has_content_type = false;

    while (std::getline(header_stream, line)) {
        if (line.empty() || line == "\r") continue;
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.erase(line.length() - 1);
        }

        if (line.find("Content-Type:") == 0 || line.find("content-type:") == 0) {
            has_content_type = true;
        }

        response += line + "\r\n";
    }
    if (!has_content_type) {
        response += "Content-Type: text/html\r\n";
    }
    std::ostringstream content_length_ss;
    content_length_ss << cgi_body.length();
    response += "Content-Length: " + content_length_ss.str() + "\r\n";
    response += "Date: ";
    response += ft_time_format();
    response += "\r\n";
    response += "Server: SpySocket/1.0\r\n";
    response += "Connection: close\r\n\r\n";
    response += cgi_body;
    return response;
}
std::string CClient::getErrorPageFromConfig(int statusCode)
{
    for (size_t i = 0; i < this->mutableConfig.errorPage.size(); ++i)
    {
        if (std::atoi(this->mutableConfig.errorPage[i].first.c_str()) == statusCode)
        {
            std::string root = this->mutableConfig.root;
            std::string errorPage = this->mutableConfig.errorPage[i].second;

            if (!root.empty() && root[root.length() - 1] != '/') {
                root += "/";
            }
            if (!errorPage.empty() && errorPage[0] == '/') {
                errorPage = errorPage.substr(1);
            }

            std::string errorPagePath = root + errorPage;
            struct stat fileStat;
            if (stat(errorPagePath.c_str(), &fileStat) != 0) {
                continue;
            }
            if (!S_ISREG(fileStat.st_mode)) {
                continue;
            }

            std::ifstream file(errorPagePath.c_str(), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                file.close();
                std::ostringstream response;
                response << "HTTP/1.1 " << statusCode << " " << getStatusMessage(statusCode) << "\r\n";
                response << "Content-Type: text/html\r\n";
                response << "Content-Length: " << buffer.str().size() << "\r\n\r\n";
                response << buffer.str();
                this->chunkedSending = true;
                return response.str();
            }
        }
    }
    this->chunkedSending = true;
    return GenerateResErr(statusCode);
}

std::string CClient::getStatusMessage(int statusCode)
{
    switch(statusCode)
    {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Content Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 429: return "Too Many Requests";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        default: return "Unknown Error";
    }
}



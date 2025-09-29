#include "../inc/CClient.hpp"
#include "../inc/Get.hpp"
#include "../inc/webserv.hpp"
#include <sstream>
#include <sys/wait.h>

CClient::CClient() :
    _name_location(""), NameMethod(""), uri(""), FdClient(-1), mutableConfig(), serv(), parser(NULL),
    SendHeader(false), readyToSendAllResponse(false), chunkedSending(false),
    chunkSize(0), bytesSent(0), response(""), filePath(""), fileSize(0),
    offset(0), fileFd(-1), intialized(false), Chunked(false),
    cgi_handler(NULL), is_cgi_request(false), cgi_headers_sent(false), cgi_body_buffer("")
{

}

CClient::CClient(std::string NameMethod, std::string uri, int FdClient, ConfigStruct MConfig, Servers* serv, ParsingRequest* parser) :
    _name_location(""), NameMethod(NameMethod), uri(uri), FdClient(FdClient), mutableConfig(MConfig), serv(serv), parser(parser),
    SendHeader(false), readyToSendAllResponse(false), chunkedSending(false), chunkSize(0), bytesSent(0),
    response(""), filePath(""), fileSize(0), offset(0), fileFd(-1), intialized(false), Chunked(false),
    cgi_handler(NULL), is_cgi_request(false), cgi_headers_sent(false), cgi_body_buffer("")

{

}

CClient::~CClient()
{
    if (cgi_handler) {
        // Ensure CGI process is properly closed before deletion
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

    // Optional: print parser address
    std::cout << "Parser ptr: " << parser << std::endl;

    // Config info (if you want details from mutableConfig)
    std::cout << "Config root: " << mutableConfig.root << std::endl;
    std::cout << "Config indexPage: " << mutableConfig.indexPage << std::endl;

    std::cout << "=======================" << std::endl;
}


std::string CClient::HandleAllMethod()
{
    // Check if this is already a CGI request in progress
    if (is_cgi_request && cgi_handler) {
        return HandleCGIMethod();
    }
    if (!cgi_handler) {
        cgi_handler = new CGI();
    }
    if (cgi_handler->check_is_cgi(*parser))
    {
        is_cgi_request = true;

        std::map<std::string, std::string> env_vars;
        if (!cgi_handler->set_env_var(env_vars, *parser)) {
            cgi_handler->close_cgi();
            delete cgi_handler;
            cgi_handler = NULL;
            is_cgi_request = false;
            return GenerateResErr(500);
        }

        bool success = false;
        if (this->NameMethod == "POST") {
            success = cgi_handler->execute_with_body(env_vars, parser->getBody());
        }
        else {
            success = cgi_handler->execute(env_vars);
        }

        if (!success) {
            int error_code = cgi_handler->get_error_code();
            cgi_handler->close_cgi();
            delete cgi_handler;
            cgi_handler = NULL;
            is_cgi_request = false;
            return GenerateResErr(error_code > 0 ? error_code : 500);
        }

        //CGI is running, will be handled in subsequent calls
        return "";
    }
    else
    {
        cgi_handler->close_cgi();
        delete cgi_handler;
        cgi_handler = NULL;
    }

    // Handle non-CGI requests as before
    if (this->NameMethod == "GET")
    {

        Get _MGet(*this);
        try {
            return (_MGet.MethodGet());
        }
        catch (const std::runtime_error& e) {
            std::string errMsg = e.what();
            if (errMsg.find("405") != std::string::npos) {
                return GenerateResErr(405);
            }
            else if (errMsg.find("403") != std::string::npos) {
                return GenerateResErr(403);
            }
            else {
                return GenerateResErr(500); // Generic server error for other exceptions
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
        return GenerateResErr(405); // Method Not Allowed
    return std::string();
}

std::string CClient::HandleCGIMethod()
{
    if (!cgi_handler) {
        return GenerateResErr(500);
    }

    // Check for CGI timeout
    if (cgi_handler->is_cgi_timeout(CGI_TIMEOUT)) {
        std::cout << "CGI timeout for client fd " << FdClient << std::endl;
        cgi_handler->close_cgi();
        delete cgi_handler;
        cgi_handler = NULL;
        is_cgi_request = false;
        return GenerateResErr(504); // Gateway Timeout
    }

    // Check if CGI process has finished
    int status;
    pid_t result = waitpid(cgi_handler->get_cgi_pid(), &status, WNOHANG);

    if (result == -1) {
        // Error occurred
        cgi_handler->close_cgi();
        delete cgi_handler;
        cgi_handler = NULL;
        is_cgi_request = false;
        return GenerateResErr(500);
    }
    else if (result == cgi_handler->get_cgi_pid()) {
        // Process has finished
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            cgi_handler->close_cgi();
            delete cgi_handler;
            cgi_handler = NULL;
            is_cgi_request = false;
            return GenerateResErr(500);
        }

        // Read any remaining output
        while (cgi_handler->read_output()) {
            // Keep reading until no more data
        }

        std::string cgi_output = cgi_handler->get_output_buffer();
        cgi_handler->close_cgi();
        delete cgi_handler;
        cgi_handler = NULL;
        is_cgi_request = false;

        // Parse CGI output and build HTTP response
        return formatCGIResponse(cgi_output);
    }
    else {
        // Process still running - read available output
        cgi_handler->read_output();
        return ""; // Return empty - not ready yet
    }
}

std::string CClient::formatCGIResponse(const std::string& cgi_output)
{
    if (cgi_output.empty()) {
        return GenerateResErr(500);
    }

    // Find the end of headers (double CRLF)
    size_t headers_end = cgi_output.find("\r\n\r\n");
    if (headers_end == std::string::npos) {
        headers_end = cgi_output.find("\n\n");
        if (headers_end == std::string::npos) {
            return GenerateResErr(500);
        }
        headers_end += 2;
    }
    else {
        headers_end += 4;
    }

    std::string cgi_headers = cgi_output.substr(0, headers_end);
    std::string cgi_body = cgi_output.substr(headers_end);

    // Build HTTP response
    std::string response = "HTTP/1.1 200 OK\r\n";

    // Parse and add CGI headers
    std::istringstream header_stream(cgi_headers);
    std::string line;
    bool has_content_type = false;

    while (std::getline(header_stream, line)) {
        if (line.empty() || line == "\r") continue;

        // Remove trailing \r if present
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.erase(line.length() - 1);
        }

        if (line.find("Content-Type:") == 0 || line.find("content-type:") == 0) {
            has_content_type = true;
        }

        response += line + "\r\n";
    }

    // Add default content-type if not present
    if (!has_content_type) {
        response += "Content-Type: text/html\r\n";
    }

    // Add content-length
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




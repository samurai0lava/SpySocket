#include "../../inc/webserv.hpp"

std::string intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

//standard HTTP status phrase
std::string getStatusPhrase(int errorCode) {
    switch (errorCode) {
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

//HTML error page generation
std::string generateErrorPageHTML(int errorCode, const std::string& errorMessage) {
    std::string statusPhrase = getStatusPhrase(errorCode);
    
    std::string html = "<!DOCTYPE html>\n";
    html += "<html>\n";
    html += "<head>\n";
    html += "    <title>" + intToString(errorCode) + " " + statusPhrase + "</title>\n";
    html += "</head>\n";
    html += "<body>\n";
    html += "    <h1>" + intToString(errorCode) + " " + statusPhrase + "</h1>\n";
    html += "    <p>" + errorMessage + "</p>\n";
    html += "    <hr>\n";
    html += "    <div>Webserver-42</div>\n";
    html += "</body>\n";
    html += "</html>\n";
    
    return html;
}

void ResERROR(const int ErrorStat)
{
    std::string statusPhrase = getStatusPhrase(ErrorStat);
    std::cerr << "[ERROR] " << ErrorStat << " " << statusPhrase << std::endl;
}

const std::string GenerateResErr(const int ErrorStat)
{
    std::string statusPhrase = getStatusPhrase(ErrorStat);
    std::string errorMessage = getDefaultErrorMessage(ErrorStat);
    std::string htmlBody;
    std::string response = "HTTP/1.1 " + intToString(ErrorStat) + " " + statusPhrase + "\r\n";
    response += "Content-Type: text/html; charset=UTF-8\r\n";
    response += "Content-Length: " + intToString(htmlBody.length()) + "\r\n";
    response += "Connection: close\r\n";
    response += "Server: SpySocket\r\n";
    response += "\r\n";
    // if (access("www/html/404.html", F_OK) == -1)
    // {
        htmlBody = generateErrorPageHTML(ErrorStat, errorMessage);
        response += htmlBody;
    // }
    // else
    // {
    //     std::ifstream error_page("www/html/404.html");
    //     while(std::getline(error_page, htmlBody))
    //     {
    //         response += htmlBody;
    //     }
    // // }
    return response;
}

std::string getDefaultErrorMessage(int errorCode) {
    switch (errorCode) {
        case 400:
            return "The request could not be understood by the server due to malformed syntax.";
        case 403:
            return "The server understood the request, but is refusing to fulfill it.";
        case 404:
            return "The requested resource could not be found on this server.";
        case 405:
            return "The method specified in the request is not allowed for the resource.";
        case 413:
            return "The request entity is larger than limits defined by server.";
        case 414:
            return "The request URI is longer than the server can interpret.";
        case 415:
            return "The media type is not supported by the server.";
        case 429:
            return "Too many requests have been made in a given amount of time.";
        case 500:
            return "The server encountered an internal error and was unable to complete your request.";
        case 501:
            return "The server does not support the functionality required to fulfill the request.";
        case 502:
            return "The server received an invalid response from the upstream server.";
        case 503:
            return "The server is temporarily unable to service your request due to maintenance.";
        case 504:
            return "The server did not receive a timely response from the upstream server.";
        case 505:
            return "The server does not support the HTTP protocol version used in the request.";
        default:
            return "An error occurred while processing your request.";
    }
}
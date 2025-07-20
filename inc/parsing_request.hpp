#ifndef PARSING_REQUEST_HPP
#define PARSING_REQUEST_HPP

#include <iostream>
#include <string>
#include <map>
#include <exception>

// class BadRequestException : public std::exception
// {
//     private:
//         std::string message;
//     public:
//         BadRequestException(const std::string& msg) : message(msg) {}
//         virtual const char* what() const throw() { return message.c_str(); }
//         virtual ~BadRequestException() throw() {}
// };

// class NotImplementedException : public std::exception
// {
//     private:
//         std::string message;
//     public:
//         NotImplementedException(const std::string& msg) : message(msg) {}
//         virtual const char* what() const throw() { return message.c_str(); }
//         virtual ~NotImplementedException() throw() {}
// };


class ParsingRequest
{

private:
    // std::string request_method;
    // std::string request_uri;
    // std::string request_version;
    std::map<std::string, std::string> start_line;
    std::map<std::string, std::string> headers;

    int connection_status; // 0 for closed, 1 for keep-alive
    int content_lenght_exists; // 0 for no content length, 1 for exists
    int transfer_encoding_exists; // 0 for no transfer encoding, 1 for exists



public:
    ParsingRequest() : connection_status(1), content_lenght_exists(0), transfer_encoding_exists(0) {}
    std::string get_start_line(const std::string& request);
    std::map<std::string, std::string> split_start_line(const std::string& start_line);
    std::string get_header_fields(const std::string& request);
    std::map<std::string, std::string> split_header(const std::string& headers);
    std::map<std::string, std::string> handle_request(const std::string& request);
    bool checkMethod(const std::map<std::string, std::string>& start_line);
    bool checkURI(const std::map<std::string, std::string>& start_line);
    bool checkVersion(const std::map<std::string, std::string>& start_line);

    std::map<std::string, std::string> getStartLine() const { return start_line; }
    std::map<std::string, std::string> getHeaders() const { return headers; }
    int getConnectionStatus() const { return connection_status; }
    int getContentLengthExists() const { return content_lenght_exists; }
    int getTransferEncodingExists() const { return transfer_encoding_exists; }
    bool checkTransferEncoding(const std::map<std::string, std::string>& headers);
    bool checkContentLength(const std::map<std::string, std::string>& headers);
    bool checkConnection(const std::map<std::string, std::string>& headers);
    bool checkContentType(const std::map<std::string, std::string>& headers);
    bool checkHost(const std::map<std::string, std::string>& headers);
};

void printMap(const std::map<std::string, std::string>& m);
void logError(int status, const std::string& error_message);

#endif
#ifndef PARSING_REQUEST_HPP
#define PARSING_REQUEST_HPP

#include <iostream>
#include <string>
#include <map>

class ParsingRequest
{

private:
    // std::string request_method;
    // std::string request_uri;
    // std::string request_version;
    std::map<std::string, std::string> start_line;
    std::map<std::string, std::string> headers;

public:
    std::string get_start_line(const std::string &request);
    std::map<std::string, std::string> split_start_line(const std::string &start_line);
    std::string get_header_fields(const std::string &request);
    std::map<std::string, std::string> split_header(const std::string &headers);
    std::map<std::string, std::string> handle_request(const std::string &request);
    // std::string getMethod() const { return request_method; }
    // std::string getURI() const { return request_uri; }
    // std::string getVersion() const { return request_version; }
    bool checkMethod(const std::map<std::string, std::string> &start_line);
    bool checkURI(const std::map<std::string, std::string> &start_line);
    bool checkVersion(const std::map<std::string, std::string> &start_line);

    std::map<std::string, std::string> getStartLine() const { return start_line; }
    std::map<std::string, std::string> getHeaders() const { return headers; }
};

#endif
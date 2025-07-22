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
    // Parsing states
    enum ParseState {
        PARSE_START_LINE,
        PARSE_HEADERS,
        PARSE_BODY,
        PARSE_COMPLETE,
        PARSE_ERROR
    };

private:
    // std::string request_method;
    // std::string request_uri;
    // std::string request_version;
    std::map<std::string, std::string> start_line;
    std::map<std::string, std::string> headers;

    int connection_status; // 0 for closed, 1 for keep-alive
    int content_lenght_exists; // 0 for no content length, 1 for exists
    int transfer_encoding_exists; // 0 for no transfer encoding, 1 for exists
    int host_exists; // 0 for no host, 1 for exists
    
    // State machine variables
    ParseState current_state;
    std::string buffer; // Accumulates incoming data
    size_t buffer_pos; // Current position in buffer
    size_t expected_body_length;
    std::string body_content;
    
    // Internal parsing methods
    bool parse_start_line();
    bool parse_headers();
    bool parse_body();
    bool find_crlf(size_t& pos);
    bool has_complete_line();

public:
    // Constructor
    ParsingRequest() : connection_status(1), content_lenght_exists(0), 
                      transfer_encoding_exists(0), host_exists(0),
                      current_state(PARSE_START_LINE), buffer_pos(0), 
                      expected_body_length(0) {}
    
    // NGINX-style incremental parsing
    enum ParseResult {
        PARSE_OK,        // Parsing successful
        PARSE_AGAIN,     // Need more data
        PARSE_ERROR_400, // Bad request
        PARSE_ERROR_501  // Not implemented
    };
    
    ParseResult feed_data(const char* data, size_t len);
    ParseResult get_parse_status() const;
    bool is_complete() const { return current_state == PARSE_COMPLETE; }
    void reset(); // Reset parser state for new request
    
    std::map<std::string, std::string> getStartLine() const { return start_line; }
    std::map<std::string, std::string> getHeaders() const { return headers; }
    std::string getBody() const { return body_content; }
    int getConnectionStatus() const { return connection_status; }
    int getContentLengthExists() const { return content_lenght_exists; }
    int getTransferEncodingExists() const { return transfer_encoding_exists; }
    int getHostExists() const { return host_exists; }
    bool checkTransferEncoding(const std::map<std::string, std::string>& headers);
    bool checkContentLength(const std::map<std::string, std::string>& headers);
    bool checkConnection(const std::map<std::string, std::string>& headers);
    bool checkContentType(const std::map<std::string, std::string>& headers);
    bool checkHost(const std::map<std::string, std::string>& headers);
};

void printMap(const std::map<std::string, std::string>& m);
void logError(int status, const std::string& error_message);

#endif
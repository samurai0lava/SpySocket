#ifndef PARSING_REQUEST_HPP
#define PARSING_REQUEST_HPP

#include <iostream>
#include <string>
#include <map>
#include <exception>


class ParsingRequest
{
public:
    enum ParseState {
        PARSE_START_LINE,
        PARSE_HEADERS,
        PARSE_BODY,
        PARSE_COMPLETE,
        PARSE_ERROR
    };

    enum ParseResult {
        PARSE_OK,         // Parsing successful
        PARSE_AGAIN,      // Need more data
        PARSE_ERROR_RESULT,
        PARSE_ERROR_400,  // Bad request
        PARSE_ERROR_403,  // Forbidden
        PARSE_ERROR_404,  // Not found
        PARSE_ERROR_405,  // Method not allowed
        PARSE_ERROR_413,  // Content Too Large
        PARSE_ERROR_414,  // Request-URI too long
        PARSE_ERROR_415,  // Unsupported Media Type
        PARSE_ERROR_429,  // Too Many Requests
        PARSE_ERROR_500,  // Internal Server Error
        PARSE_ERROR_501,  // Not implemented
        PARSE_ERROR_502,  // Bad Gateway
        PARSE_ERROR_503,  // Service Unavailable
        PARSE_ERROR_504,  // Gateway Timeout
        PARSE_ERROR_505,  // HTTP Version Not Supported
    };
protected:
    std::map<std::string, std::string> start_line;
    std::map<std::string, std::string> headers;

    int connection_status;
    int content_lenght_exists;
    int transfer_encoding_exists;
    int host_exists;
    int error_code;
    std::string error_message;
    int status_code;
    std::string status_phrase;
    std::string boundary;
    ParseState current_state;
    ParseResult result_p;
    std::string buffer;
    size_t buffer_pos;
    size_t expected_body_length;
    std::string body_content;
    std::string query_string;
    size_t chunked_last_processed_size;
    std::string chunked_accumulated_data;
    bool parse_start_line();
    bool parse_headers();
    bool parse_body();
    bool find_crlf(size_t& pos);

public:
    ParsingRequest() : connection_status(1), content_lenght_exists(0),
        transfer_encoding_exists(0), host_exists(0),
        current_state(PARSE_START_LINE), buffer_pos(0),
        expected_body_length(0), chunked_last_processed_size(0) {
    }
    ParseResult feed_data(const char* data, size_t len);
    ParseResult get_parse_status() const;
    bool is_complete() const { return current_state == PARSE_COMPLETE; }
    void reset();

    std::map<std::string, std::string> getStartLine() const { return start_line; }
    std::map<std::string, std::string> getHeaders() const { return headers; }
    std::string getBody() const { return body_content; }
    int getConnectionStatus() const { return connection_status; }
    int getContentLengthExists() const { return content_lenght_exists; }
    int getTransferEncodingExists() const { return transfer_encoding_exists; }
    int getHostExists() const { return host_exists; }
    int getErrorCode() const { return error_code; }
    std::string getErrorMessage() const { return error_message; }
    int getStatusCode() const { return status_code; }
    std::string getStatusPhrase() const { return status_phrase; }
    std::string getQueryString() const { return query_string; }
    std::string getCookies() const;
    std::string getId() const;
    std::string generateSetCookieHeader(const std::string& name, const std::string& value) const;
    bool checkTransferEncoding(const std::map<std::string, std::string>& headers);
    bool checkURI(std::string& uri);
    bool checkVersion(const std::string& verion);
    bool checkContentLength(const std::map<std::string, std::string>& headers);
    bool checkConnection(const std::map<std::string, std::string>& headers);
    bool checkContentType(const std::map<std::string, std::string>& headers);
    bool checkMethod(const std::string& method);
    bool checkHost(const std::map<std::string, std::string>& headers);
    bool checkLocation(const std::map<std::string, std::string>& headers);
    bool checkCookie(const std::map<std::string, std::string>& headers);
};

void printMap(const std::map<std::string, std::string>& m);
void logError(int status, const std::string& error_message);
void printRequestInfo(const ParsingRequest& request, int fd);
void handle_signal(void);
bool is_path_secure(const std::string &fullPath);

#endif

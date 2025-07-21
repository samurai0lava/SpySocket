#include "inc/webserv.hpp"

void test_request(std::string request, ParsingRequest& parsingRequest)
{
    if (parsingRequest.handle_request(request))
    {
        std::cout << BLUE "Parsed Request Start Line:" RESET << std::endl;
        std::map<std::string, std::string> start_line = parsingRequest.getStartLine();
        printMap(start_line);
        std::cout << BLUE "Parsed Request Headers:" RESET << std::endl;
        std::map<std::string, std::string> headers = parsingRequest.getHeaders();
        printMap(headers);
    }

}

void print_status(ParsingRequest& parsingRequest)
{
    std::cout << YELLOW "Connection Status: " RESET << parsingRequest.getConnectionStatus() << std::endl;
    std::cout << YELLOW "Content Length Exists: " RESET << parsingRequest.getContentLengthExists() << std::endl;
    std::cout << YELLOW "Transfer Encoding Exists: " RESET << parsingRequest.getTransferEncodingExists() << std::endl;
    std::cout << YELLOW "Host Exists: " RESET << parsingRequest.getHostExists() << std::endl;
}

int main()
{

    std::string request_dummy = "GET /index.html HTTP/1.1\r\n"
        // "HOST: LOCALHOST:8080\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Connection: keep-alive\r\n\r\n";

    std::string request_valid = "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Connection: keep-alive\r\n\r\n";

    std::string request_not_implemented = "PUT /api/users HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 45\r\n\r\n"
        "{\"name\":\"Updated User\",\"age\":25}";

    std::string request_dummy2 = "POST /api/users HTTP/1.1\r\n"
        "Host:        localhost\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36\r\n"
        "Accept: application/json, text/plain, */*\r\n"
        "Accept-Language: en-US,en;q=0.9,fr;q=0.8\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 85\r\n"
        "Origin: https://example.com\r\n"
        "Referer: https://example.com/register\r\n"
        "Cookie: session_id=abc123; csrf_token=xyz789\r\n"
        "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9\r\n"
        "X-Requested-With: XMLHttpRequest\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n\r\n"
        "{\"name\":\"John Doe\",\"email\":\"john@example.com\",\"password\":\"secretpass123\"}";

    std::string request_post = "POST /api/users HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36\r\n"
        "Accept: application/json, text/plain, */*\r\n"
        "Accept-Language: en-US,en;q=0.9,fr;q=0.8\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Content-Type: application/json\r\n"
        "Transfer-EncoDing: chunked\r\n"
        "Origin: https://example.com\r\n"
        "Referer: https://example.com/register\r\n"
        "Cookie: session_id=abc123; csrf_token=xyz789\r\n"
        "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9\r\n"
        "X-Requested-With: XMLHttpRequest\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n\r\n"
        "55\r\n"
        "{\"name\":\"John Doe\",\"email\":\"john@example.com\",\"password\":\"secretpass123\"}\r\n"
        "0\r\n"
        "\r\n";


    ParsingRequest request;

    test_request(request_dummy, request);
    std::cout << std::endl;
    print_status(request);
    std::cout << std::endl;
    test_request(request_valid, request);
    std::cout << std::endl;
    print_status(request);
    std::cout << std::endl;
    test_request(request_not_implemented, request);
    std::cout << std::endl;
    print_status(request);
    std::cout << std::endl;
    test_request(request_dummy2, request);
    print_status(request);
    std::cout << std::endl;
    test_request(request_post, request);
    print_status(request);
    return 0;
}
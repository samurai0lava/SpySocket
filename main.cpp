#include "inc/webserv.hpp"

int main()
{

	std::string request_dummy = "GET /index.html HTTP/1.1\r\n"
								"Host: localhost:8080\r\n"
								"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
								"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
								"Accept-Language: en-US,en;q=0.5\r\n"
								"Accept-Encoding: gzip, deflate\r\n"
								"Connection: keep-alive\r\n\r\n";

	// Real-world POST request example (form submission or API call)
	std::string request_dummy2 = "POST /api/users HTTP/1.1\r\n"
								 "Host: localhost:8080\r\n"
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

	ParsingRequest request;

	// Test the first request (GET)
	std::cout << BLUE "========== Testing GET Request ==========" RESET << std::endl;
	std::string start_line_str = request.get_start_line(request_dummy);
	std::map<std::string, std::string> start_line = request.split_start_line(start_line_str);

	if (!request.checkMethod(start_line) || !request.checkURI(start_line) || !request.checkVersion(start_line))
	{
		std::cerr << "Invalid request format." << std::endl;
		return 1;
	}

	std::cout << BLUE "Request Method: " RESET << start_line["method"] << std::endl;
	std::cout << BLUE "Request URI: " RESET << start_line["uri"] << std::endl;
	std::cout << BLUE "Request Version: " RESET << start_line["version"] << std::endl;

	std::string headers_str = request.get_header_fields(request_dummy);
	std::map<std::string, std::string> headers = request.split_header(headers_str);
	std::cout << "Headers:" << std::endl;
	
	// Print all headers
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::cout << "  " << it->first << GREEN " equal to " RESET << it->second << std::endl;
	}

	// Test the second request (POST)
	std::cout << std::endl << BLUE "========== Testing POST Request ==========" RESET << std::endl;
	std::string start_line_str2 = request.get_start_line(request_dummy2);
	std::map<std::string, std::string> start_line2 = request.split_start_line(start_line_str2);

	if (!request.checkMethod(start_line2) || !request.checkURI(start_line2) || !request.checkVersion(start_line2))
	{
		std::cerr << "Invalid request format." << std::endl;
		return 1;
	}

	std::cout << BLUE "Request Method: " RESET << start_line2["method"] << std::endl;
	std::cout << BLUE "Request URI: " RESET << start_line2["uri"] << std::endl;
	std::cout << BLUE "Request Version: " RESET << start_line2["version"] << std::endl;

	std::string headers_str2 = request.get_header_fields(request_dummy2);
	std::map<std::string, std::string> headers2 = request.split_header(headers_str2);
	std::cout << "Headers:" << std::endl;
	
	// Print all headers
	for (std::map<std::string, std::string>::const_iterator it = headers2.begin(); it != headers2.end(); ++it)
	{
		std::cout << "  " << it->first << GREEN " equal to " RESET << it->second << std::endl;
	}

	return 0;
}

#include "inc/webserv.hpp"

int main()
{

	// Invalid request with typo (should trigger BadRequestException)
	std::string request_dummy = "GeT /index.html HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Connection: keep-alive\r\n\r\n";

	// Valid GET request
	std::string request_valid = "GET /index.html HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Connection: keep-alive\r\n\r\n";

	// Not implemented method (should trigger NotImplementedException)
	std::string request_not_implemented = "PUT /api/users HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 45\r\n\r\n"
		"{\"name\":\"Updated User\",\"age\":25}";

	// Real-world POST request example (form submission or API call)
	std::string request_dummy2 = "POST /api/users HTTP/1.1\r\n"
		"Host :        localhost\r\n"
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
	std::map<std::string, std::string> parsed_request;
	
	try {
		parsed_request = request.handle_request(request_dummy2);
		std::cout << BLUE "Parsed Request Start Line:" RESET << std::endl;
		printMap(parsed_request);
		std::cout << BLUE "Parsed Request Headers:" RESET << std::endl;
		printMap(request.getHeaders());
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	try
	{
		parsed_request = request.handle_request(request_valid);
		std::cout << BLUE "Parsed Request Start Line:" RESET << std::endl;
		printMap(parsed_request);
		std::cout << BLUE "Parsed Request Headers:" RESET << std::endl;
		printMap(request.getHeaders());
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	try {
		parsed_request = request.handle_request(request_not_implemented);
		std::cout << std::endl;
		std::cout << BLUE "Parsed Request Start Line:" RESET << std::endl;
		printMap(parsed_request);
		std::cout << BLUE "Parsed Request Headers:" RESET << std::endl;
		printMap(request.getHeaders());
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

}